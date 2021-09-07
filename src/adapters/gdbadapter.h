#pragma once
#include "../debugadapter.h"
#include "../debugadaptertype.h"
#include "rspconnector.h"
#include <map>

class GdbAdapter : public DebugAdapter
{
protected:
    struct RegisterInfo
    {
        std::uint32_t m_bitSize{};
        std::uint32_t m_regNum{};
        std::uint32_t m_offset{};
    };

    DebugStopReason m_lastStopReason{};

    using register_pair = std::pair<std::string, RegisterInfo>;
    std::map<std::string, DebugRegister> m_cachedRegisterInfo{};

    Socket* m_socket;
    RspConnector m_rspConnector{};

    std::map<std::string, RegisterInfo> m_registerInfo{};

    std::uint32_t m_internalBreakpointId{};
    std::vector<DebugBreakpoint> m_debugBreakpoints{};

    std::uint32_t m_lastActiveThreadId{};

    std::string ExecuteShellCommand(const std::string& command);
    virtual bool LoadRegisterInfo();

    uint64_t m_currentIP;

    bool m_redirectGDBServer;

    std::recursive_mutex m_mutex;


    virtual DebugStopReason SignalToStopReason(std::uint64_t signal);

public:
    GdbAdapter(bool redirectGDBServer = true);
    ~GdbAdapter();

    bool Execute(const std::string& path) override;
    bool ExecuteWithArgs(const std::string& path, const std::vector<std::string>& args) override;
    bool Attach(std::uint32_t pid) override;
    bool Connect(const std::string& server, std::uint32_t port) override;

    void Detach() override;
    void Quit() override;

    std::vector<DebugThread> GetThreadList() override;
    DebugThread GetActiveThread() const override;
    std::uint32_t GetActiveThreadId() const override;
    bool SetActiveThread(const DebugThread& thread) override;
    bool SetActiveThreadId(std::uint32_t tid) override;

    DebugBreakpoint AddBreakpoint(std::uintptr_t address, unsigned long breakpoint_type = 0) override;
    std::vector<DebugBreakpoint> AddBreakpoints(const std::vector<std::uintptr_t>& breakpoints) override;
    bool RemoveBreakpoint(const DebugBreakpoint& breakpoint) override;
    bool RemoveBreakpoints(const std::vector<DebugBreakpoint>& breakpoints) override;
    bool ClearAllBreakpoints() override;
    std::vector<DebugBreakpoint> GetBreakpointList() const override;
    bool BreakpointExists(uint64_t address) const;

    std::string GetRegisterNameByIndex(std::uint32_t index) const override;
    std::unordered_map<std::string, DebugRegister> ReadAllRegisters() override;
    DebugRegister ReadRegister(const std::string& reg) override;
    bool WriteRegister(const std::string& reg, std::uintptr_t value) override;
    bool WriteRegister(const DebugRegister& reg, std::uintptr_t value) override;
    std::vector<std::string> GetRegisterList() const override;

    bool ReadMemory(std::uintptr_t address, void* out, std::size_t size) override;
    bool WriteMemory(std::uintptr_t address, const void* out, std::size_t size) override;
    std::string GetRemoteFile(const std::string& path);
    std::vector<DebugModule> GetModuleList() override;

    std::string GetTargetArchitecture() override;

    uint64_t GetCurrentIP() const { return m_currentIP; }
    void SetCurrentIP(uint64_t address) { m_currentIP = address; }

    DebugStopReason StopReason() override;
    unsigned long ExecStatus() override;

    bool GenericGo(const std::string& go_type);
    bool GenericGoAsync(const std::string& go_type);

    bool BreakInto() override;
    bool Go() override;
    bool StepInto() override;
    bool StepOver() override;
    bool StepTo(std::uintptr_t address) override;

    void Invoke(const std::string& command) override;
    std::uintptr_t GetInstructionOffset() override;

    bool SupportFeature(DebugAdapterCapacity feature) override;

    void SchedulerThread();
};


class GdbAdapterType: public DebugAdapterType
{
public:
    GdbAdapterType();
    virtual DebugAdapter* Create(BinaryNinja::BinaryView* data);
    virtual bool IsValidForData(BinaryNinja::BinaryView* data);
    virtual bool CanExecute(BinaryNinja::BinaryView* data);
    virtual bool CanConnect(BinaryNinja::BinaryView* data);
};

void InitGdbAdapterType();
