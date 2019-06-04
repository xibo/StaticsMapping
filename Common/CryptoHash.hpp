#pragma once

#include <QtCore/QByteArray>
#include <QtCore/QCryptographicHash>
#include <QtCore/QMap>
#include <QtCore/QJsonValue>
#include <QtCore/QSharedPointer>
#include <QtCore/QSet>
#include <QtCore/QString>

struct Hash
{
  using Algorithm = QCryptographicHash::Algorithm;
  using Algorithms = QSet<Algorithm>;
  using Results = QMap<Algorithm, QByteArray>;
  using BackendPointer = QSharedPointer<QCryptographicHash>;
  using Backends = QMap<Algorithm, BackendPointer>;
  
  Hash();
  Hash(const QJsonValue &json);
  Hash(QByteArray data);
  ~Hash();
  
  QJsonObject toJson() const;
  QString toString() const;
  
  void addData(Algorithm algo, QByteArray data);
  void addResult(Algorithm algo, QByteArray hash);
  void addResult(Algorithm algo, const QJsonValue &hash);
  QByteArray result(Algorithm algo) const;
  
  bool hasResult(Algorithm algo) const;
  Algorithms results() const;
  QString resultString(Algorithm algo) const;
  
  static QByteArray hash(QByteArray data, Algorithm algo);
  static Algorithms requiredMethods();
  static Algorithms preferedMethods();
  static Algorithm keyMethod();
  
  bool operator== (const Hash &other) const;
  
protected:
  Results m_results;
  Backends m_backends;
  
protected:
  static Algorithms g_required;
  static Algorithms g_prefered;
  static Algorithm g_key;
};
