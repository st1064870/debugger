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

#include "attachbinaryview.h"
#include "QFileDialog"

using namespace BinaryNinjaDebuggerAPI;
using namespace BinaryNinja;
using namespace std;

AttachBinaryViewDialog::AttachBinaryViewDialog(QWidget* parent, DebuggerController* controller): QDialog()
{
    setWindowTitle("Attach Binary View");
    setMinimumSize(UIContext::getScaledWindowSize(400, 130));
    setAttribute(Qt::WA_DeleteOnClose);

    m_controller = controller;

    setModal(true);
    auto* layout = new QVBoxLayout;
    layout->setSpacing(0);

    m_pathEntry = new QLineEdit(this);
	m_pathEntry->setMinimumWidth(800);
    m_baseEntry = new QLineEdit(this);

	auto* pathSelector = new QPushButton("...", this);
	pathSelector->setMaximumWidth(30);
	connect(pathSelector, &QPushButton::clicked, [&](){
		auto fileName = QFileDialog::getOpenFileName(this, "Select Executable Path", m_pathEntry->text());
		if (!fileName.isEmpty())
			m_pathEntry->setText(fileName);
	});

	auto pathEntryLayout = new QHBoxLayout;
	pathEntryLayout->addWidget(m_pathEntry);
	pathEntryLayout->addWidget(pathSelector);

	auto* contentLayout = new QVBoxLayout;
	contentLayout->setSpacing(10);
	contentLayout->addWidget(new QLabel("Executable/Database Path"));
	contentLayout->addLayout(pathEntryLayout);
	contentLayout->addWidget(new QLabel("Base Address"));
	contentLayout->addWidget(m_baseEntry);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);

    QPushButton* cancelButton = new QPushButton("Cancel");
    connect(cancelButton, &QPushButton::clicked, [&](){ reject(); });
    QPushButton* acceptButton = new QPushButton("Accept");
    connect(acceptButton, &QPushButton::clicked, [&](){ apply(); });
    acceptButton->setDefault(true);

    buttonLayout->addStretch(1);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(acceptButton);

    layout->addLayout(contentLayout);
    layout->addStretch(1);
    layout->addSpacing(10);
    layout->addLayout(buttonLayout);
    setLayout(layout);
}


void AttachBinaryViewDialog::apply()
{
    std::string path = m_pathEntry->text().toStdString();
	if (path.empty())
		return;

	Ref<BinaryView> view = OpenView(path, false);

    std::string baseString = m_baseEntry->text().toStdString();
    uint64_t remoteBase;
    try
    {
        remoteBase = stoull(baseString, nullptr, 16);
    }
    catch(const std::exception&)
    {
        remoteBase = 0;
    }

	auto typeName = view->GetTypeName();
	auto file = view->GetFile();
	if (view->GetStart() != remoteBase)
	{
		file->Rebase(view, remoteBase);
	}

	Ref<BinaryView> rebasedView = file->GetViewOfType(typeName);
	m_controller->GetLiveView()->GetFile()->AttachBinaryView(rebasedView, "Debugger");

    accept();
}
