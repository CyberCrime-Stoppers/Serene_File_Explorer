#pragma once
/*
 * Serene File Explorer - Properties dialog (General + Permissions tabs)
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <QDialog>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;

class PropertiesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PropertiesDialog(const QString &path, QWidget *parent = nullptr);

private slots:
    void applyPermissions();

private:
    void buildGeneralTab(const QFileInfo &fi);
    void buildPermissionsTab(const QFileInfo &fi);

    QString m_path;

    // General tab
    QLabel *m_iconLabel    = nullptr;
    QLabel *m_nameLabel    = nullptr;
    QLabel *m_typeLabel    = nullptr;
    QLabel *m_sizeLabel    = nullptr;
    QLabel *m_locationLabel = nullptr;
    QLabel *m_modifiedLabel = nullptr;
    QLabel *m_accessedLabel = nullptr;

    // Permissions tab
    QCheckBox  *m_ownerRead  = nullptr;
    QCheckBox  *m_ownerWrite = nullptr;
    QCheckBox  *m_ownerExec  = nullptr;
    QCheckBox  *m_groupRead  = nullptr;
    QCheckBox  *m_groupWrite = nullptr;
    QCheckBox  *m_groupExec  = nullptr;
    QCheckBox  *m_otherRead  = nullptr;
    QCheckBox  *m_otherWrite = nullptr;
    QCheckBox  *m_otherExec  = nullptr;
    QCheckBox  *m_executable = nullptr;
    QLineEdit  *m_targetEdit = nullptr;
    QPushButton *m_applyBtn  = nullptr;
};
