#ifndef STANZA_H
#define STANZA_H

#include <QMetaType>
#include <QSharedData>
#include <QDomDocument>
#include "utilsexport.h"

class StanzaData :
	public QSharedData
{
public:
	StanzaData(const QString &ATagName);
	StanzaData(const QDomElement &AElem);
	StanzaData(const StanzaData &AOther);
public:
	QDomDocument FDoc;
};

class UTILS_EXPORT Stanza
{
public:
	Stanza(const QString &ATagName = "message");
	Stanza(const QDomElement &AElem);
	~Stanza();
	void detach();
	bool isValid() const;
	bool isFromServer() const;
	QDomDocument document() const;
	QDomElement element() const;
	QString attribute(const QString &AName) const;
	Stanza &setAttribute(const QString &AName, const QString &AValue);
	QString tagName() const;
	Stanza &setTagName(const QString &ATagName);
	QString type() const;
	Stanza &setType(const QString &AType);
	QString id() const;
	Stanza &setId(const QString &AId);
	QString to() const;
	Stanza &setTo(const QString &ATo);
	QString from() const;
	Stanza &setFrom(const QString &AFrom);
	QString lang() const;
	Stanza &setLang(const QString &ALang);
	QDomElement firstElement(const QString &ATagName = QString::null, const QString &ANamespace = QString::null) const;
	QDomElement addElement(const QString &ATagName, const QString &ANamespace = QString::null);
	QDomElement createElement(const QString &ATagName, const QString &ANamespace = QString::null);
	QDomText createTextNode(const QString &AData);
	QString toString(int AIndent = 1) const;
	QByteArray toByteArray() const;
public:
	static bool isValidXmlChar(quint32 ACode);
	static QString replaceInvalidXmlChars(QString &AXml, const QChar &AWithChar='?');
	static QDomElement findElement(const QDomElement &AParent, const QString &ATagName = QString::null, const QString &ANamespace = QString::null);
private:
	QSharedDataPointer<StanzaData> d;
};

Q_DECLARE_METATYPE(Stanza);

#endif // STANZA_H
