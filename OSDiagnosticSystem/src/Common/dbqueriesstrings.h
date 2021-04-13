#ifndef DBQUERIESSTRINGS_H
#define DBQUERIESSTRINGS_H
#include <QString>

/** @brief Хранение всех строк запросов с параметризацией для работы с БД (SQLite) */

//----- Запросы ControllerUsers -----

///----- INSERT -----
const QString InsertUserQuery_str =
    "INSERT INTO users (name) VALUES (:p_name)";
const QString InsertOperationQuery_str =
    "INSERT INTO operations (name) VALUES (:p_name)";
const QString InsertUserPasswordQuery_str =
    "INSERT INTO passwords_states (user_id, operation_date, op_id, pswd) VALUES (:p_user_id, :p_operation_date, :p_op_id, :p_pswd)";

///----- SELECT -----
const QString GetUserIdQuery_str =
    "SELECT user_id FROM users WHERE name = :p_name;";
const QString GetOperationIdQuery_str =
    "SELECT operation_id FROM operations WHERE name = :p_name;";
const QString GetOperationNameQuery_str =
    "SELECT name FROM operations WHERE operation_id = :p_name;";
const QString GetLastPasswordQuery_str =
    "SELECT p.pswd\
    FROM passwords_states p\
    JOIN users u ON u.user_id = p.user_id\
    WHERE u.user_id = (SELECT u1.user_id\
                       FROM users u1\
                       WHERE u1.name = :p_name\
    )\
    AND p.passwords_states_id = (SELECT MAX(p1.passwords_states_id)\
                                 FROM passwords_states p1\
                                 WHERE p1.user_id = u.user_id)";
//    const QString GetLastOperationQuery_str =
//    "SELECT t.operation_date, t.user_id FROM passwords_states WHERE user_id = :p_user_id";
    const QString GetLastOperationQuery_str =
    "SELECT * FROM passwords_states WHERE user_id = :p_user_id";
///----- DELETE -----
const QString DeleteUserQuery_str =
    "DELETE FROM users WHERE name = :p_name";
///----- UPDATE -----
const QString UpdateOperationDateQuery_str =
    "UPDATE passwords_states "
    "SET operation_date = :p_op_date, op_id = :p_op_id, pswd = :p_pswd "
    "WHERE user_id = :p_user_id";


#endif // DBQUERIESSTRINGS_H
