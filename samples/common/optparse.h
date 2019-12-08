#ifndef SPH_OPTPARSE_H
#define SPH_OPTPARSE_H

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace sph {
namespace cmd {

/**
 * @brief Single command line option.
 */
struct Option {
    /// name of the option (e.g. "capture", always required)
    std::string name;
    /// shortname (e.g. "c", optional)
    std::string shortname;
    /// description, optional
    std::string description;
    /// whether this option requires an argument
    bool arg = false;
    /// whether this option is required
    bool required = false;
};

/**
 * Callback type for functions to be executed during the option parsing process.
 */
using OptionParserCallback = std::function<void(const std::string&)>;

/**
 * @brief Simple getopt like option parser.
 */
class OptionParser {
public:
    OptionParser() = default;

    /**
     * @brief Add an option to the parser.
     * @param opt The option to add.
     */
    void add(const Option &opt) {
        for (const auto &opt_ : m_opts) {
            if (opt_.name == opt.name || opt_.shortname == opt.shortname) {
                throw std::logic_error("Option name and shortname must be unique");
            }
        }

        m_opts.push_back(opt);
    }

    /**
     * @brief Add an option to the parser.
     * @param opt The option to add.
     * @param cb Callback to be executed when the option is found.
     */
    void add(const Option &arg, OptionParserCallback cb) {
        add(arg);
        m_callbacks.emplace_back(std::make_pair(arg, cb));
    }

    /**
     * @brief Parse an option string.
     * @param input String of tuples.
     * @return Options mapped to their values.
     */
    std::map<std::string, std::string> parse(const std::string &input) {
        std::string _input = input;
        std::map<std::string, std::string> opts;
        size_t start = 0;
        size_t end;
        std::string name_or_shortname;

        // ensure there is a leading whitespace char before the first arg
        _input.insert(0, " ");

        while (true) {
            Option current;

            // get the option
            start = _input.find(" -", start);
            if (start == std::string::npos) {
                break;
            }

            start++;
            while (_input[start] == '-') {
                start++;
            }

            end = _input.find(' ', start);
            if (end == std::string::npos) {
                end = _input.length();
            }

            // found the option (or its shortname)
            name_or_shortname = _input.substr(start, end - start);

            // check our arg table for validity
            for (const auto &opt : m_opts) {
                if (name_or_shortname == opt.name || name_or_shortname == opt.shortname) {
                    current = opt;
                    opts[current.name] = "";
                    break;
                }
            }

            if (current.name.empty()) {
                throw std::runtime_error(std::string("Unrecognized option: " + name_or_shortname));
            }

            // now get the value, if any
            start = ++end;
            end = _input.find(" -", start);
            if (end == std::string::npos) {
                end = _input.length();
            }

            if (end > start) {
                // read the value as string
                opts[current.name] = _input.substr(start, end - start);
            }

            // check whether the opt requires an argument
            if (opts[current.name].empty() && current.arg) {
                throw std::runtime_error(std::string("Argument required for opt: " + current.name));
            }

            // execute parser callbacks, if any
            for (const auto &cb : m_callbacks) {
                if (cb.first.name == current.name) {
                    cb.second(opts[current.name]);
                }
            }
        }

        // check whether all required args are present
        for (const auto &opt : m_opts) {
            if (opt.required && (opts.find(opt.name) == opts.end())) {
                throw std::runtime_error(std::string("Missing required argument: ") + opt.name);
            }
        }

        return opts;
    }

    /**
     * @brief Parse command line arguments.
     * @param argc Number of options.
     * @param argv Command line options.
     * @return Options mapped to their values.
     */
    std::map<std::string, std::string> parse(int argc, char **argv) {
        std::string args;
        for (int i = 1; i < argc; i++) {
            args += std::string(argv[i]);
            if (i < argc - 1) {
                args += ' ';
            }
        }

        return parse(args);
    }

    /**
     * @brief Generate help text from parser options.
     * @param description Whether to print the description string.
     * @return List of strings.
     */
    std::vector<std::string> help(bool description = false) const {
        std::vector<std::string> ret;
        size_t max_name_len = 0;
        size_t max_shortname_len = 0;
        size_t max_desc_len = 0;

        for (const auto &opt : m_opts) {
            if (max_name_len < opt.name.length()) {
                max_name_len = opt.name.length();
            }
            if (max_shortname_len < opt.shortname.length()) {
                max_shortname_len = opt.shortname.length();
            }
            if (max_desc_len < opt.description.length()) {
                max_desc_len = opt.description.length();
            }
        }

        // small helper
        auto pad = [](const std::string &str, size_t num, char padchar = ' ', bool back = true) {
            if (num <= str.length()) {
                return str;
            }

            std::string copy(str);
            copy.insert(back ? str.length() : 0, num - str.length(), padchar);
            return copy;
        };

        for (const auto &opt : m_opts) {
            std::string optstr;

            optstr += std::string("    ");
            if (!opt.shortname.empty()) {
                optstr += std::string("-");
            }
            optstr += pad(opt.shortname, max_shortname_len);

            optstr += std::string("  ");
            if (!opt.name.empty()) {
                optstr += std::string("--");
            }
            optstr += pad(opt.name, max_name_len);

            if (description) {
                optstr += std::string("  ");
                optstr += opt.description;
            }

            ret.push_back(optstr);
        }

        return ret;
    }

private:
    /// The parser options specified by the developer.
    std::vector<Option> m_opts;

    /// Callbacks executed during option parsing.
    std::vector<std::pair<Option, OptionParserCallback>> m_callbacks;
};

} // namespace cmd
} // namespace sph

#endif // SPH_OPTPARSE_H
