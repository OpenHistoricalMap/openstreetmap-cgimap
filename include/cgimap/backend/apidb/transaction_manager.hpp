#ifndef TRANSACTION_MANAGER_HPP
#define TRANSACTION_MANAGER_HPP

#include "cgimap/logger.hpp"

#include <chrono>
#include <set>
#include <fmt/core.h>
#include <pqxx/pqxx>

#include <iostream>


class Transaction_Owner_Base
{
public:
  Transaction_Owner_Base() = default;
  virtual pqxx::transaction_base& get_transaction() = 0;
  virtual std::set<std::string>& get_prep_stmt() = 0;
  virtual ~Transaction_Owner_Base() = default;
};


class Transaction_Owner_ReadOnly : public Transaction_Owner_Base
{
public:
  explicit Transaction_Owner_ReadOnly(pqxx::connection &conn, std::set<std::string> &prep_stmt);
  pqxx::transaction_base& get_transaction() override;
  std::set<std::string>& get_prep_stmt() override;
  ~Transaction_Owner_ReadOnly() override = default;

private:
  pqxx::read_transaction m_txn;
  std::set<std::string>& m_prep_stmt;
};


class Transaction_Owner_ReadWrite : public Transaction_Owner_Base
{
public:
  explicit Transaction_Owner_ReadWrite(pqxx::connection &conn, std::set<std::string> &prep_stmt);
  pqxx::transaction_base& get_transaction() override;
  std::set<std::string>& get_prep_stmt() override;
  ~Transaction_Owner_ReadWrite() override = default;

private:
  pqxx::work m_txn;
  std::set<std::string>& m_prep_stmt;
};

class Transaction_Owner_Void : public Transaction_Owner_Base
{
public:
  explicit Transaction_Owner_Void();
  pqxx::transaction_base& get_transaction() override;
  std::set<std::string>& get_prep_stmt() override;
  ~Transaction_Owner_Void() override = default;
};



class Transaction_Manager {

public:
  explicit Transaction_Manager(Transaction_Owner_Base &to);

  void prepare(const std::string &name, const std::string &);

  pqxx::result exec(const std::string &query,
                    const std::string &description = std::string());
  void commit();

  template<typename... Args>
  pqxx::result exec_prepared(const std::string &statement, Args&&... args) {

    auto start = std::chrono::steady_clock::now();
    pqxx::result res = m_txn.exec_prepared(statement, std::forward<Args>(args)...);

    auto end = std::chrono::steady_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    logger::message(fmt::format("Executed prepared statement {} in {:d} ms, returning {:d} rows, {:d} affected rows",
                               statement,
                               elapsed.count(),
			       res.size(),
			       res.affected_rows()));
    return res;
  }

private:
  pqxx::transaction_base & m_txn;
  std::set<std::string>& m_prep_stmt;
};


#endif
