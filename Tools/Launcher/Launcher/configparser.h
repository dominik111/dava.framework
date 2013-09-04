#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include "defines.h"
#include <yaml-cpp/yaml.h>
#include <QString>
#include <QMap>
#include <QVector>
#include <QSet>

QString GetStringValueFromYamlNode(const YAML::Node * node, QString defaultValue = "");

class ConfigParser;

struct AppVersion
{
    QString id;
    QString runPath;
    QString cmd;
    QString url;

    static AppVersion LoadFromYamlNode(const YAML::Node * node);
};

struct Application
{
    Application() {}
    Application(const QString & _id) : id(_id) {}

    QString id;

    int GetVerionsCount() {return versions.size();}
    AppVersion * GetVersion(int index) {return &versions[index];}
    AppVersion * GetVersion(const QString & versionID);

    void RemoveVersion(const QString & versionID);

    static Application LoadFromYamlNode(const YAML::Node * node);

private:
    QVector<AppVersion> versions;

    friend class ConfigParser;
};

struct Branch
{
    Branch() {}
    Branch(const QString & _id) : id(_id) {}

    QString id;

    int GetAppCount() {return applications.size();}
    Application * GetApplication(int index) {return &applications[index];}
    Application * GetApplication(const QString & appID);

    void RemoveApplication(const QString & appID);

    static Branch LoadFromYamlNode(const YAML::Node * node);

private:
    QVector<Application> applications;

    friend class ConfigParser;
};

class ConfigParser
{
public:
    ConfigParser(const QByteArray & configData);

    void SaveToYamlFile(const QString & filePath);

    void InsertApplication(const QString & branchID, const QString & appID, const AppVersion & version);
    void RemoveApplication(const QString & branchID, const QString & appID, const QString & version);

    int GetBranchCount();
    QString GetBranchID(int index);

    Branch * GetBranch(int branchIndex);
    Branch * GetBranch(const QString &branch);
    Application * GetApplication(const QString &branch, const QString &appID);
    AppVersion * GetAppVersion(const QString &branch, const QString &appID, const QString &ver);

    void RemoveBranch(const QString & branchID);

    const QString & GetString(const QString & stringID);

    void SetLauncherURL(const QString & url);
    void SetWebpageURL(const QString & url);
    void SetRemoteConfigURL(const QString & url);
    void SetLastNewsID(const QString & id);

    const QString & GetLauncherVersion();
    const QString & GetLauncherURL();
    const QString & GetWebpageURL();
    const QString & GetRemoteConfigURL();
    const QString & GetNewsID();

    void MergeBranchesIDs(QSet<QString> & branches);

    void CopyStringsFromConfig(const ConfigParser & parser);

private:
    QString launcherVersion;
    QString launcherURL;
    QString webPageURL;
    QString remoteConfigURL;
    QString newsID;

    QVector<Branch> branches;
    QMap<QString, QString> strings;
};

#endif // CONFIGPARSER_H