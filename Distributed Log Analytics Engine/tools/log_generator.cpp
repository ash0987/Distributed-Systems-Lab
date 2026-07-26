#include "log_generator.h"
#include <fstream>
#include <iostream>
#include <cstdlib>

std::vector<std::string> load_templates(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERROR: could not open template file at: " << path << "\n";
        std::exit(1);
    }
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}

std::string replace_placeholder(std::string tmpl, const std::string& placeholder, const std::string& value) {
    size_t pos = tmpl.find(placeholder);
    if (pos != std::string::npos) {
        tmpl.replace(pos, placeholder.length(), value);
    }
    return tmpl;
}

std::vector<std::string> get_templates_for_service()
{
    return load_templates("templates.txt");
}

std::string fill_template(std::string tmpl, std::mt19937& rng, const std::string& service_name, const std::string& host_id)
{
    std::uniform_int_distribution<int> n_dist(1, 500);
    tmpl = replace_placeholder(tmpl, "{n}", std::to_string(n_dist(rng)));

    std::uniform_int_distribution<int> ms_dist(1, 2000);
    tmpl = replace_placeholder(tmpl, "{ms}", std::to_string(ms_dist(rng)));

    std::uniform_int_distribution<int> pct_dist(0, 100);
    tmpl = replace_placeholder(tmpl, "{pct}", std::to_string(pct_dist(rng)));

    std::uniform_int_distribution<int> id_dist(1, 10000);
    tmpl = replace_placeholder(tmpl, "{id}", "req-" + std::to_string(id_dist(rng)));

    std::vector<std::string> codes = {"200", "400", "404", "500", "503"};
    std::uniform_int_distribution<size_t> code_dist(0, codes.size() - 1);
    tmpl = replace_placeholder(tmpl, "{code}", codes[code_dist(rng)]);

    std::vector<std::string> op = {"sync","backup","reindex","compaction"};
    std::uniform_int_distribution<size_t> op_dist(0, op.size() - 1);
    tmpl = replace_placeholder(tmpl, "{op}", op[op_dist(rng)]);

    std::uniform_int_distribution<int> user_dist(1, 1000000);
    tmpl = replace_placeholder(tmpl, "{user}", "user-" + std::to_string(user_dist(rng)));

    std::uniform_int_distribution<int> key_dist(1, 1000000);
    tmpl = replace_placeholder(tmpl, "{key}", "key-" + std::to_string(key_dist(rng)));

    tmpl = replace_placeholder(tmpl, "{service}", service_name);
    tmpl = replace_placeholder(tmpl, "{host}", host_id);

    return tmpl;
}

void generate_logs(size_t count, std::ostream& out, std::mt19937& rng)
{
    std::map<std::string, std::vector<std::string>> host_pools;

    std::map<std::string, int> service_host_counts = {
        {"data-node", 10000},
        {"metadata-store", 1000},
        {"api", 500},
        {"kms", 150},
        {"auth", 150}
    };

    for (const auto& [service, host_count] : service_host_counts) {
        std::vector<std::string> hosts;
        for (int i = 0; i < host_count; i++) {
            hosts.push_back(service + "-" + std::to_string(i));
        }
        host_pools[service] = hosts;
    }

    // Load templates once per service, not per record
    std::map<std::string, std::vector<std::string>> service_templates;
    for (const auto& [service, count] : service_host_counts) {
        service_templates[service] = get_templates_for_service();
    }

    std::vector<std::string> service_names = {"data-node", "metadata-store", "api", "kms", "auth"};
    std::vector<double> weights = {10000, 1000, 500, 150, 150};
    std::discrete_distribution<int> service_dist(weights.begin(), weights.end());

    std::map<std::string, uint64_t> seq_no_map;

    std::vector<std::string> levels = {"INFO", "WARN", "ERROR"};
    std::vector<double> level_weights = {90, 5, 5};
    std::discrete_distribution<int> level_dist(level_weights.begin(), level_weights.end());

    std::uniform_int_distribution<int> thread_dist(1, 64);
    std::uniform_int_distribution<int> process_dist(1000, 9999);

    auto now = std::chrono::system_clock::now();
    std::uniform_int_distribution<int64_t> time_dist(0, 5 * 24 * 60 * 60);

    for (size_t i = 0; i < count; i++) {
        int service_idx = service_dist(rng);
        std::string service = service_names[service_idx];

        auto& hosts = host_pools[service];
        std::uniform_int_distribution<size_t> host_dist(0, hosts.size() - 1);
        std::string host = hosts[host_dist(rng)];

        uint64_t seq = ++seq_no_map[host];

        int thread_id = thread_dist(rng);
        int process_id = process_dist(rng);

        std::string level = levels[level_dist(rng)];

        auto& templates = service_templates[service];
        std::uniform_int_distribution<size_t> tmpl_dist(0, templates.size() - 1);
        std::string message = fill_template(templates[tmpl_dist(rng)], rng, service, host);

        auto offset_seconds = time_dist(rng);
        auto timestamp = now - std::chrono::seconds(offset_seconds);
        std::time_t time_value = std::chrono::system_clock::to_time_t(timestamp);

        std::stringstream timestamp_stream;
        timestamp_stream << std::put_time(std::gmtime(&time_value), "%Y-%m-%dT%H:%M:%SZ");

        out << timestamp_stream.str() << "|" << host << "|" << service << "|" << seq << "|"
            << thread_id << "|" << process_id << "|" << level << "|" << message << "\n";
    }
}