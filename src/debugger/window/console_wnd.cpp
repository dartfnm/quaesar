#include "console_wnd.h"
#include <debugger/debugger.h>
#include <imgui_eastl.h>
#include <src/generic/thread.h>
#include <src/log.h>


namespace qd::window {

class ConsoleLogWriter : public ILogWriter {
    eastl::vector<LogEntry> msgList;
    qd::thread::Mutex mMutex;

public:
    struct EntriesList;
    // friend struct EntriesList;

public:
    EntriesList getEntriesList();

protected:
    virtual void addLogEntry(const LogEntry& entry) override;

    virtual ~ConsoleLogWriter() = default;

};  // class ConsoleLogWriter
//////////////////////////////////////////////////////////////////////////


struct ConsoleLogWriter::EntriesList {
    ConsoleLogWriter* mOwner;
    qd::thread::Mutex* mpMutex;

public:
    EntriesList(ConsoleLogWriter* in_owner) : mOwner(in_owner), mpMutex(&mOwner->mMutex) {
        mpMutex->lock();
    }

    EntriesList(EntriesList&& rh) : mOwner(rh.mOwner), mpMutex(eastl::move(rh.mpMutex)) {
        rh.mpMutex = nullptr;
    }
    const LogEntry* begin() const {
        return &*(mOwner->msgList.begin());
    }

    const LogEntry* end() const {
        return &*(mOwner->msgList.end());
    }

    ~EntriesList() {
        if (mpMutex) {
            mpMutex->unlock();
            mpMutex = nullptr;
        }
    }
};  // struct EnriesList
//////////////////////////////////////////////////////////////////////////


void ConsoleWnd::drawContent() {
    ImVec2 rgn = ImGui::GetContentRegionAvail();
    ImVec2 scrollingChildSize = ImVec2(rgn.x, rgn.y - ImGui::GetTextLineHeightWithSpacing());
    if (ImGui::BeginChild("##scrolling", scrollingChildSize, ImGuiChildFlags_None,
                          ImGuiWindowFlags_HorizontalScrollbar)) {
        ConsoleLogWriter::EntriesList list = mpConsoleWriter->getEntriesList();
        for (const LogEntry& curEnt : list) {
            ImGui::TextUnformatted(curEnt.message.c_str());
        }
    }
    ImGui::EndChild();

    // constole input
    ImGui::TextUnformatted("> ");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::InputText("##input", &inputStr, ImGuiInputTextFlags_EnterReturnsTrue)) {
        getDbg()->execConsoleCmd(inputStr.c_str());
        inputStr.clear();
    }
}


void ConsoleWnd::onCreate(UiViewCreate* cp) {
    UiWindow::onCreate(cp);
    mTitle = "Console";
    mpConsoleWriter = qd::logConsole().createWriter_<ConsoleLogWriter>();
}


void ConsoleWnd::destroy() {
    logConsole().destroyWriter(mpConsoleWriter);
    mpConsoleWriter = nullptr;

    TSuper::destroy();
}


void ConsoleLogWriter::addLogEntry(const LogEntry& entry) {
    mMutex.lock();
    msgList.push_back(entry);
    mMutex.unlock();
}


qd::window::ConsoleLogWriter::EntriesList ConsoleLogWriter::getEntriesList() {
    return EntriesList(this);
}


};  // namespace qd::window
