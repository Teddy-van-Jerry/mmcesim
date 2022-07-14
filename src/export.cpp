#include "export.h"

Export::Export(const CLI_Options& opt) : _opt(opt) {
    std::tie(_config,_errors) = ReadConfig::read(opt.input);
    _setLang();
    if (_opt.output == "__UNDEFINED") {
        std::filesystem::path input_path = _opt.input;
        _opt.output = input_path.replace_extension(_langExtension()).string();
        if (!_opt.force && !std::filesystem::exists(_opt.output)) {
            YAML_Error e(Err::OUTPUT_FILE_EXISTS);
            _errors.push_back(e);
            _already_error_before_export = true;
        }
    }
    _f_ptr = new std::ofstream(_opt.output);
    if (!_f().is_open()) {
        YAML_Error e(Err::CANNOT_OPEN_OUTPUT_FILE);
        _errors.push_back(e);
        _already_error_before_export = true;
    }
}

Export::Export(const CLI_Options& opt, const YAML::Node& config, const YAML_Errors& errors) 
    : _opt(opt), _config(config), _errors(errors) {
    _setLang();
    if (_opt.output == "__UNDEFINED") {
        std::filesystem::path input_path = _opt.input;
        _opt.output = input_path.replace_extension(_langExtension()).string();
        if (!_opt.force && !std::filesystem::exists(_opt.output)) {
            YAML_Error e(Err::OUTPUT_FILE_EXISTS);
            _errors.push_back(e);
            _already_error_before_export = true;
        }
    }
    _f_ptr = new std::ofstream(_opt.output);
    if (!_f().is_open()) {
        YAML_Error e(Err::CANNOT_OPEN_OUTPUT_FILE);
        _errors.push_back(e);
        _already_error_before_export = true;
    }
}

Export::~Export() {
    delete _f_ptr;
}

YAML_Errors Export::exportCode() {
    if (_already_error_before_export) return _errors;
    // do something
    _topComment();
    _f_ptr->close();
    return _errors;
}

YAML_Errors Export::exportCode(const CLI_Options& opt) {
    Export ep(opt);
    return ep.exportCode();
}

YAML_Errors Export::exportCode(const CLI_Options& opt, const YAML::Node& config, const YAML_Errors& errors) {
    Export ep(opt, config, errors);
    return ep.exportCode();
}

bool Export::_preCheck(const YAML::Node& n, unsigned a_t, bool mattered) {
    bool find_match = false;
    if (a_t & DType::INT && n.IsScalar()) {
        find_match = true;
        try { n.as<int>(); } catch(...) { find_match = false; }
    }
    if (!find_match && a_t & DType::DOUBLE && n.IsScalar()) {
        find_match = true;
        try { n.as<double>(); } catch(...) { find_match = false; }
    }
    if (!find_match && a_t & DType::STRING && n.IsScalar()) {
        find_match = true;
        try { n.as<std::string>(); } catch(...) { find_match = false; }
    }
    if (!find_match && a_t & DType::BOOL && n.IsScalar()) {
        find_match = true;
        try { n.as<bool>(); } catch(...) { find_match = false; }
    }
    if (!find_match && a_t & DType::CHAR && n.IsScalar()) {
        find_match = true;
        try { n.as<char>(); } catch(...) { find_match = false; }
    }
    if (!find_match && a_t & DType::SEQ && n.IsSequence()) {
        find_match = true;
    }
    if (!find_match && a_t & DType::MAP && n.IsMap()) {
        find_match = true;
    }
    if (!find_match && a_t & DType::NUL && n.IsNull()) {
        find_match = true;
    }
    if (!find_match && a_t & DType::UNDEF && !n.IsDefined()) {
        find_match = true;
    }
    if (find_match) return true;
    else {
        if (mattered) {
            // This is the general error message,
            // but in use, the error message can be more specific
            // by using method _setLatestError.
            std::string msg = errorMsg(Err::YAML_DTYPE) + " (" + n.Tag() + ")";
            YAML_Error e(msg, Err::YAML_DTYPE);
            _errors.push_back(e);
        }
        return false;
    }
}

void Export::_setLang() {
    _info(std::string("This is mmCEsim ") + _MMCESIM_VER_STR
        + ", with target version " + _config["version"].as<std::string>() + ".");
    if (auto n = _config["simulation"]; _preCheck(n, DType::MAP | DType::UNDEF)) {
        try {
            std::string lang_str = _as<std::string>(n["backend"]);
            boost::algorithm::to_lower(lang_str);
            if (lang_str == "cpp" || lang_str == "c++") lang = Lang::CPP;
            if (lang_str == "matlab" || lang_str == "m") lang = Lang::MATLAB;
            if (lang_str == "octave" || lang_str == "gnu octave") lang = Lang::OCTAVE;
            if (lang_str == "py" || lang_str == "python") lang = Lang::PY;
            if (lang_str == "ipynb") lang = Lang::IPYNB;
        } catch(...) {
            _setLatestError("'simulation->backend' is not a string"
                " (should be \"cpp\", \"matlab\", \"octave\", \"py\" or \"ipynb\").");
        }
    } else {
        _setLatestError("No simulation block defined.");
        _already_error_before_export = true;
    }
    _info("Set simulation backend as " + _langName() + ".");
}

void Export::_topComment() {
    // TODO: ipynb settings
    std::string title;
    try {
        title = _asStr(_config["meta"]["title"], false);
        if (title == "") throw("Title empty!");
    } catch(...) {
        title =  _opt.output ; // TODO: Only file name.
    }
    _wComment() << "Title: " << title << '\n';
    try {
        std::string desc = _asStr(_config["meta"]["description"], false);
        _wComment() << "Description: " << desc << '\n';
    } catch(...) {}
    try {
        std::string author = _asStr(_config["meta"]["author"], false);
        _wComment() << "Author: " << author << '\n';
    } catch(...) {}

    // get the current time
    std::time_t curr_time = std::time(nullptr);
    std::tm     curr_tm   = *std::localtime(&curr_time);
    const char* time_format = "%F %T (UTC %z)";

    _wComment() << "Date: " << std::put_time(&curr_tm, time_format) << "\n";
    _wComment() << '\n';
    _wComment() << "This file is auto generated using " << _MMCESIM_NAME << ' ' << _MMCESIM_VER_STR << ",\n";
    _wComment() << "With initial target as version " << _config["_compiled"]["version_str"] << ".\n";
    _wComment() << '\n';
    _wComment() << _MMCESIM_NAME << " is open source under the " << _MMCESIM_LIC << ".\n";
    _wComment() << "GitHub organization at " << _MMCESIM_GIT << ".\n";
    _wComment() << "Web app is available at " << _MMCESIM_WEBAPP << ".\n";
    _wComment() << "Visit " << _MMCESIM_WEB << " for more information.\n";
    if (lang == Lang::CPP) {
        _wComment() << '\n';
        _wComment() << "Compile Commands:\n";
        _wComment() << "$ g++ " << _opt.output << " -std=c++11 -larmadillo\n";
        _wComment() << "or\n";
        _wComment() << "$ clang++ " << _opt.output << " -std=c++11 -larmadillo\n";
        _wComment() << "or just link to Armadillo library with whatever compiler you have.\n";
    }
    _f() << "\n";
}
