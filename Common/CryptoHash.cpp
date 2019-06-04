#include <limits.h>

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QMetaEnum>

#include "CryptoHash.hpp"

Hash::Algorithms Hash::g_required({QCryptographicHash::Keccak_256});
Hash::Algorithms Hash::g_prefered({QCryptographicHash::Keccak_256, QCryptographicHash::Sha256});
Hash::Algorithm  Hash::g_key(QCryptographicHash::Keccak_256);

Hash::Hash ()
: m_results{}
{
}


Hash::Hash(QByteArray data)
: Hash()
{
  Q_FOREACH(Algorithm method, preferedMethods() + requiredMethods()) {
    m_results.insert(method, hash(data, method));
  }
}


Hash::Hash(const QJsonValue &json)
: Hash()
{
  QMetaEnum meta_enum(QMetaEnum::fromType<QCryptographicHash::Algorithm>());
  if (json.isObject()) {
    QJsonObject obj(json.toObject());
    Q_FOREACH(QString method, obj.keys()) {
      int method_id(meta_enum.keyToValue(qUtf8Printable(method)));
      if(method_id != -1) {
        addResult((Algorithm)method_id, obj[method]);
      }
    }
  } else {
    addResult(keyMethod(), json);
  }
}


Hash::~Hash ()
{
}


QJsonObject Hash::toJson() const
{
  QJsonObject retval;
  QMetaEnum meta_enum(QMetaEnum::fromType<QCryptographicHash::Algorithm>());
  
  Q_FOREACH(Algorithm algo, m_results.keys()) {
    retval.insert(meta_enum.valueToKey(algo), QString::fromUtf8(m_results[algo].toHex()));
  }
  
  return retval;
}


QString Hash::toString() const
{
  return QJsonDocument(toJson()).toJson();
}

Hash::Algorithms Hash::results() const
{
  return m_results.keys().toSet();
}

QByteArray Hash::result(Algorithm method) const
{
  return m_results.value(method);
}

QString Hash::resultString(Algorithm method) const
{
  return QString::fromUtf8(result(method).toHex());
}

bool Hash::hasResult(Algorithm method) const
{
  return m_results.contains(method);
}

void Hash::addData(Algorithm method, QByteArray data)
{
  if(!m_backends.contains(method)) {
    m_backends.insert(method, BackendPointer(new QCryptographicHash(method)));
  }
  m_backends[method]->addData(data);
  m_results[method] = m_backends[method]->result();
}

bool Hash::addResult(Algorithm method, QByteArray value)
{
  bool write(true);
  if (m_results.contains(method)) {
    if (m_results.value(method) == value) {
      write = false;
    }
  }
  if (write) {
    m_results.insert(method, value);
  }
  return (write);
}

bool Hash::addResult(const Hash &other)
{
  Q_FOREACH(Algorithm method, requiredMethods()) {
    if (this->hasResult(method) && other.hasResult(method)) {
      if (this->result(method) != other.result(method)) {
        throw std::runtime_error("keyMethod() dismatch");
      }
    }
  }
  
  bool retval(false);
  Q_FOREACH(Algorithm method, other.results()) {
    if (addResult(method, other.result(method))) {
      retval = true;
    }
  }
  return (retval);
}

void Hash::addResult(Algorithm method, const QJsonValue &value)
{
  addResult(method, QByteArray::fromHex(value.toString().toUtf8()));
}

QByteArray Hash::hash(QByteArray data, Algorithm algo)
{
  return QCryptographicHash::hash(data, algo);
}

Hash::Algorithms Hash::requiredMethods()
{
  Q_ASSERT(g_required.contains(keyMethod()));
  return g_required;
}

Hash::Algorithms Hash::preferedMethods()
{
  Q_ASSERT(g_required.contains(keyMethod()));
  return g_prefered;
}

Hash::Algorithm Hash::keyMethod()
{
  return g_key;
}


bool Hash::operator== (const Hash &other) const
{
  QSet<Algorithm> mine(m_results.keys().toSet());
  QSet<Algorithm> others(other.m_results.keys().toSet());
  QSet<Algorithm> common(mine.intersect(others));
  if (common.size() != 0) {
    Q_FOREACH(Algorithm algo, common) {
      if (m_results[algo] != other.m_results[algo]) {
        return (false);
      }
    }
    return (true);
  }
  return (false);
}
