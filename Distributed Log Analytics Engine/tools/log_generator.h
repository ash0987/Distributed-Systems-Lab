#include <map>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>

// Returns ~30 message templates for a given service, with {placeholder} markers.
std::vector<std::string> get_templates_for_service();

// Picks one template and replaces its {placeholders} with randomized values.
std::string fill_template(std::string tmpl, std::mt19937& rng, const std::string& service_name, const std::string& host_id);

// Generates `count` log records and writes them as lines to `out`.
void generate_logs(size_t count, std::ostream& out, std::mt19937& rng);
