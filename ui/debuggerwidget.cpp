/*
Copyright 2020-2022 Vector 35 Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <QPainter>
#include <QHeaderView>
#include <QLineEdit>
#include <QStatusBar>
#include "debuggerwidget.h"
#include "ui.h"

using namespace BinaryNinjaDebuggerAPI;
using namespace BinaryNinja;
using namespace std;


DebuggerWidget::DebuggerWidget(const QString& name, ViewFrame* view, BinaryViewRef data):
    SidebarWidget(name), m_view(view), m_data(data)
{
    m_controller = DebuggerController::GetController(m_data);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->setAlignment(Qt::AlignTop);

    m_splitter = new QSplitter(Qt::Vertical, this);
    m_splitter->setChildrenCollapsible(true);

    m_controlsWidget = new DebugControlsWidget(this, "Controls", data);
    m_registersWidget = new DebugRegistersWidget("Debugger Registers", m_view, m_data);
    m_breakpointsWidget = new DebugBreakpointsWidget("Debugger Breakpoints", m_view, m_data, m_menu);
    m_modulesWidget = new DebugModulesWidget("Debugger Modules", m_view, m_data, m_menu);

    m_splitter->addWidget(m_controlsWidget);
    m_splitter->addWidget(m_registersWidget);
    m_splitter->addWidget(m_breakpointsWidget);
    m_splitter->addWidget(m_modulesWidget);

    layout->addWidget(m_splitter);
    setLayout(layout);

	m_ui = DebuggerUI::GetForViewFrame(view);
	connect(m_ui, &DebuggerUI::debuggerEvent, this, &DebuggerWidget::uiEventHandler);
}


DebuggerWidget::~DebuggerWidget()
{
}


void DebuggerWidget::notifyFontChanged()
{
    LogWarn("font changed");
}


void DebuggerWidget::updateContent()
{
    m_registersWidget->updateContent();
    m_modulesWidget->updateContent();
}


void DebuggerWidget::uiEventHandler(const DebuggerEvent &event)
{
	m_controlsWidget->updateButtons();
    switch (event.type)
    {
    case TargetStoppedEventType:
        // These updates ensure the widgets become empty after the target stops
    case DetachedEventType:
    case QuitDebuggingEventType:
    case BackEndDisconnectedEventType:
		updateContent();
		break;
	case ActiveThreadChangedEvent:
		// The registers are thread-related
		m_registersWidget->updateContent();
		break;
    case RelativeBreakpointAddedEvent:
    case AbsoluteBreakpointAddedEvent:
    case RelativeBreakpointRemovedEvent:
    case AbsoluteBreakpointRemovedEvent:
		m_breakpointsWidget->updateContent();
		break;
    default:
        break;
    }
}
