#pragma once

#include <wfp_killer.h>
#include <cxxopts.h>

namespace wfpk {
class CliCommand
{
public:
    // Non-owning pointer to a WfpKiller instance
    CliCommand(wfpk::WfpKiller *pWfpKiller)
    : _pWfpKiller{pWfpKiller}
    {
        assert(_pWfpKiller);
    }

    virtual ~CliCommand() = default;

    CliCommand(const CliCommand&) = delete;
    CliCommand &operator=(const CliCommand&) = delete;

public:
    void run(int argc, char **argv) { runCommand(argc, argv); }
    std::string help() const { return _pOptions ? _pOptions->help() : ""; }
    std::string description() const { return _pOptions ? _pOptions->description() : ""; }

private:
    // NVI idiom
    virtual void runCommand(int argc, char **argv) = 0;

protected:
    // Wrappers around _pOptions
    void initOptions(std::string name, std::string helpString)
    {
        _pOptions.emplace(name, helpString);
    }

    template <typename... Types>
    void addOption(Types&&... args)
    {
        assert(_pOptions);
        _pOptions->add_options()(std::forward<Types>(args)...);
    }

    auto parseOptions(int argc, char **argv)
    {
        assert(_pOptions);
        return _pOptions->parse(argc, argv);
    }

protected:
    wfpk::WfpKiller *_pWfpKiller{nullptr};

private:
    std::optional<cxxopts::Options> _pOptions;
};
}
