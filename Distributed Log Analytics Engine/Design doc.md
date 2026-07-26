# Distributed Log Analytics Engine

## 1. Problem Statement

Modern cloud infrastructure generates telemetry at a scale that has outgrown naive log-handling
approaches. A single production incident can require correlating events across thousands of hosts
and services, but as raw log volume climbs into the hundreds of millions of records, sequential
scanning becomes both too slow to support real-time triage and too storage-expensive to retain at
scale uncompressed. Existing centralized logging solutions often trade off ingestion throughput
against query latency, or require operators to over-provision storage to keep historical data
searchable. This project addresses that gap by building a log analytics engine that treats
high-throughput ingestion, compressed storage, and low-latency distributed search as co-equal
design constraints rather than sequential afterthoughts.

## 2. Introduction

This project implements a distributed log analytics engine, inspired by cloud observability
platforms (Elasticsearch, Loki, CloudWatch Logs Insights), in C++. It ingests structured log
records at high throughput, stores them in compressed, time-bucketed segments, builds an inverted
index for fast term search, and answers boolean/regex queries over a simulated distributed cluster
using a KQL-like query language.

The system is built as a single-machine simulation of a distributed cluster: multiple shard
"nodes" run as separate threads/processes communicating over local sockets, each owning a subset
of the data determined by consistent hashing. This keeps the project buildable and benchmarkable
solo, while preserving the real distributed-systems mechanics (partitioning, scatter-gather query
execution, parallel speedup) that a true multi-machine deployment would need.

A distinguishing design choice in this project is treating shard allocation as a **resource-quota
problem tied to cost attribution**: services are allocated ring capacity (virtual nodes) in
proportion to an admin-configured weight, so that a service's storage/compute footprint maps
directly to a billable quota — reflecting how real multi-tenant systems attribute infrastructure
cost per tenant.

Out of scope: true multi-machine/networked deployment (simulated via local processes instead),
live data migration on shard-count change, dynamic/adaptive quota rebalancing (static,
admin-configured quotas only), and cross-host causal ordering beyond per-source sequence numbers
(no vector clocks).

## 3. Goals & Non-Goals

### Functional goals
- Log ingestion (high-throughput, durable via WAL)
- Compression (block compression + dictionary encoding)
- Inverted indexing (per-segment, merged)
- Distributed search (simulated cluster, consistent hashing)
- KQL-like query language (boolean, field filters, regex)
- Parallel query execution (scatter-gather across shards)

### Benchmark targets

| Metric                 | Target                    |
| ----------------------- | -------------------------- |
| Log Dataset             | 100M+ log records          |
| Ingestion Throughput    | 150–300 MB/s               |
| Index Build Time        | <60 sec for 100M logs      |
| Search Latency          | <300 ms                    |
| Boolean Query Latency   | <500 ms                    |
| Regex Query Latency     | <700 ms                    |
| Compression Ratio       | 50–65%                     |
| Index Size              | <25% of raw data           |
| Parallel Query Speedup  | 3–5×                       |
| Concurrent Queries      | 500+                       |

### Non-goals
- Live shard rebalancing / data migration on cluster resize
- Dynamic, feedback-driven quota adjustment (static/admin-set only)
- True multi-machine networked deployment
- Full causal ordering across hosts (vector clocks) — approximate ordering via per-source sequence
  numbers is accepted as sufficient for debugging use cases

## 4. Log Record Schema

| Field          | Type    | Purpose                                                                 |
| -------------- | ------- | ------------------------------------------------------------------------ |
| `timestamp`    | uint64  | Event time (ms since epoch), primary ordering key                       |
| `host_id`      | string  | Originating machine, part of the ordering source key                    |
| `service_name` | string  | Logical service/tenant; drives sharding, quota, and query scoping       |
| `seq_no`       | uint64  | Monotonically increasing per-source (`host_id + service_name`) counter, breaks ties within a source when timestamps collide or clocks are imprecise |
| `level`        | enum    | INFO/WARN/ERROR/etc., commonly filtered field                           |
| `message`      | string  | Free-text payload, tokenized for the inverted index                     |

Ordering guarantee: within a single source, `(timestamp, seq_no)` gives an exact total order.
Across sources, ordering is approximate (wall-clock timestamp), which is accepted as sufficient
since true cross-host causal ordering would require vector/Lamport clocks — out of scope.

## 5. Storage Format

- Data is stored in **append-only, immutable segments**. A segment is scoped to one
  `(service_name, time_bucket)` pair (e.g., one hour of one service's logs).
- A new segment is opened when either the time bucket rolls over or a size threshold is hit.
- Each segment is block-compressed (zstd) and uses dictionary encoding for low-cardinality
  repeated fields (`service_name`, `host_id`, `level`) to reduce redundancy beyond what generic
  compression captures.
- Segment metadata (min/max timestamp, service, byte size, record count) is stored in a
  lightweight **catalog** (hash map keyed by `(service_name, time_bucket)`) so queries and the
  expiry job can find relevant segments without scanning the filesystem.
- Because a segment's time range is known up front, **TTL expiry (5 days) is a cheap bulk
  operation**: the expiry job scans the catalog, drops segments whose max timestamp is older than
  `now - TTL`, and deletes the corresponding files — no per-record deletion or compaction needed.

## 6. Sharding & Routing

- Shards are placed on a **consistent-hash ring**.
- Shard key = `hash(service_name + time_bucket)`, not `hash(service_name)` alone. This is a
  deliberate choice to avoid hot-partition skew: if a single high-volume service always hashed to
  the same shard, that shard would be permanently overloaded regardless of ring balancing. Folding
  the time bucket into the key spreads a single service's data across shards over time, since each
  new time bucket rehashes to a (potentially different) shard.
- Trade-off: a query scoped to one service across a wide time range now fans out to multiple
  shards instead of one. This is accepted because the system already implements parallel
  scatter-gather query execution as a core feature, so added fan-out is turned into a benchmarked
  strength (parallel speedup) rather than a pure cost.
- **Virtual nodes**: each physical shard is given multiple points on the ring to smooth
  distribution of many distinct `(service, time_bucket)` keys.
- **Weighted virtual nodes for quota/billing**: the number of virtual nodes assigned per service is
  admin-configurable, not uniform. A service configured for a larger quota gets proportionally more
  ring capacity (and thus more shard/storage share), tying infrastructure allocation directly to a
  billable resource quota. This is static and admin-reconfigurable — no automatic/dynamic
  rebalancing is implemented, and changing a quota does not trigger live data migration (explicitly
  a non-goal).

## 7. Ingestion Path

1. Incoming log record is parsed and validated.
2. Record is appended to a **write-ahead log (WAL)** file before being acknowledged, ensuring
   durability: if the process crashes before the in-memory buffer is flushed, the WAL can be
   replayed on restart to recover unflushed records.
3. Record is added to an in-memory per-segment buffer.
4. When the buffer's time bucket rolls over or hits a size threshold, it is compressed and flushed
   to a segment file; the corresponding WAL entries are then safely truncated.
5. Crash recovery: on restart, any WAL entries not yet reflected in a flushed segment are replayed
   to rebuild the in-memory buffer before ingestion resumes, avoiding data loss and avoiding
   double-application of already-flushed records.

## 8. Indexing

- An **inverted index** maps tokens (from tokenized `message` text, plus structured fields like
  `service_name`/`level`) to posting lists of matching record IDs.
- Posting lists are stored as **roaring bitmaps** for compact storage and fast set operations
  (intersection for AND, union for OR), which is the basis for the optimized boolean query
  performance target.
- A hash map (not a B+ tree) is used for the term → posting-list lookup, since term lookups are
  exact-match rather than range queries; a B+ tree's ordered-range advantage isn't needed here
  (prefix/wildcard queries are handled separately, e.g., via a trie or explicit prefix scan, not by
  making the whole index a B+ tree).
- Indexes are built **per segment** at flush time (so build cost is amortized/incremental, not a
  single 100M-record batch job) and can be merged across segments within a shard for broader
  queries.

## 9. Query Language & Execution

### Grammar (KQL-like), examples
- `service:auth-service AND level:ERROR`
- `service:payment-service AND (message:"timeout" OR message:"connection refused")`
- `host:host-042 AND NOT level:INFO`
- `message:/fail(ed|ure).*retry/` (regex)

Grammar sketch: queries are field filters (`field:value`) and free-text terms, combined with
`AND` / `OR` / `NOT`, grouped with parentheses, with an optional regex form. A recursive-descent
parser turns this into an AST, which is compiled into a query plan (which fields/terms to look up,
which posting lists to intersect/union, whether a regex fallback scan over matched candidates is
needed).

### Execution
1. Coordinator determines which shards can contain relevant data (using the catalog metadata:
   service + time range in the query prunes irrelevant shards/segments).
2. Query plan is sent to each relevant shard in parallel.
3. Each shard executes locally: posting-list intersection/union via roaring bitmap operations,
   regex applied only to the reduced candidate set (not a full scan).
4. Coordinator merges per-shard results (sorted by timestamp/seq_no), applies final
   pagination/ranking, and returns to the caller.
5. Parallelism (thread pool executing shard queries concurrently, plus per-shard use of
   vectorized bitmap operations) is the basis for the 3–5× parallel speedup and sub-second latency
   targets.

## 10. Expiry & Metering

- **Expiry**: a background job periodically scans the catalog for segments whose max timestamp is
  older than the configured TTL (default 5 days) and deletes them, along with their catalog
  entries. This is an O(segments-expired) bulk operation, not per-record.
- **Metering**: per-service counters track ingested bytes, post-compression stored bytes, and
  query counts. These feed the quota/billing model described in Section 6 — a service's measured
  usage is the basis an admin uses to (re)configure its ring weight/quota.

## 11. Benchmark Plan

| Metric | Measurement approach |
| --- | --- |
| Ingestion throughput | Feed synthetic generator output at max rate; measure MB/s sustained over a multi-minute run |
| Index build time | Time index construction across a 100M-record dataset already ingested |
| Search / boolean / regex latency | Query harness issuing each query type against the full dataset, measuring p50/p95 latency over many runs |
| Compression ratio | Compare raw record bytes vs. on-disk segment bytes post-compression |
| Index size | Compare total index bytes vs. raw dataset bytes |
| Parallel query speedup | Compare single-shard-equivalent sequential query time vs. full parallel scatter-gather time |
| Concurrent queries | Load-test harness issuing 500+ simultaneous queries, measuring throughput/latency degradation |

## 12. Future Work

- Dynamic, feedback-driven quota rebalancing (currently static/admin-configured only)
- True multi-machine/networked deployment (currently simulated via local processes/threads)
- Live data migration on shard-count changes
- Cross-host causal ordering via vector/Lamport clocks