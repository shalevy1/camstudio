/**
 * Copyright(C) 2018  Steven Hoving
 *
 * This program is free software : you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "settings_model.h"

class application_settings_ui : public CDialogEx
{
    DECLARE_DYNAMIC(application_settings_ui)

public:
    application_settings_ui(CWnd* pParent = nullptr);
    application_settings_ui(CWnd* pParent, settings_model &model);
    virtual ~application_settings_ui();

    BOOL OnInitDialog() override;

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_APPLICATION_SETTINGS_UI };
#endif

private:
    settings_model *settings_{nullptr};

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()
public:
    CButton minimize_on_record_checkbox_;
    CButton auto_filename_generation_checkbox_;
    CComboBox output_directory_combobox_;
    CEdit output_directory_user_specified_edit_;
    CButton output_directory_user_specified_browse_button_;
    CComboBox temp_directory_combobox_;
    CEdit temp_directory_user_specified_edit_;
    CButton temp_directory_user_specified_browse_button_;
    afx_msg void OnBnClickedApplicationMinimizeOnRecord();
    afx_msg void OnBnClickedAutoFilenameGeneration();
};