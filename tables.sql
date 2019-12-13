CREATE DATABASE IF NOT EXISTS memo_data;

-- server side

CREATE TABLE IF NOT EXISTS accounts (
    id VARCHAR(36) NOT NULL,
    pswd VARCHAR(256) NOT NULL,
    PRIMARY KEY (id)
);

-- accounts
-- LOG_IN
SELECT COUNT(*) FROM accounts WHERE id = ? AND pswd = ?;

-- LOG_OUT
-- SELECT COUNT(*) FROM accounts WHERE id = ?;

-- CREATE_ACCOUNT,
INSERT INTO accounts(id, pswd) VALUES (?, ?);

CREATE TABLE IF NOT EXISTS memos(
    memo_id INT NOT NULL,
    memo_title VARCHAR(20),
    PRIMARY KEY (memo_id)
); -- first

CREATE TABLE IF NOT EXISTS records(
    memo_id INT NOT NULL,
    record_id INT NOT NULL,
    due_date DATE,
    record_text VARCHAR(80) NOT NULL,
    PRIMARY KEY (memo_id, record_id),
    CONSTRAINT records_memo_ref FOREIGN KEY (memo_id) REFERENCES memos (memo_id) ON DELETE CASCADE
); -- second

CREATE TABLE IF NOT EXISTS id_memo(
    id VARCHAR(36) NOT NULL,
    memo_id INT NOT NULL,
    PRIMARY KEY(id, memo_id),
    CONSTRAINT id_memo_id_ref FOREIGN KEY (id) REFERENCES accounts (id),
    CONSTRAINT id_memo_memo_ref FOREIGN KEY (memo_id) REFERENCES memos (memo_id) ON DELETE CASCADE
); -- third

-- data

-- SYNC_FROM_SERVER
SELECT memo_id FROM id_memo; -- get ids

SELECT memo_title FROM memos WHERE memo_id = ?; -- get titles

SELECT record_id, due_date, record_text FROM records WHERE memo_id = ?; -- get records

-- SELECT * FROM id_memo NATURAL JOIN memos NATURAL JOIN records WHERE id = ?;

-- SYNC_FROM_CLIENT
DELETE FROM memos WHERE memo_id IN (SELECT memo_id FROM id_memo);

INSERT INTO memos VALUES (?, ?);
INSERT INTO id_memo VALUES (?, ?);
-- INSERT INTO ? VALUES (?, ?); -- BatchExec

-- memo_id, record_id, due_date, record_text
INSERT INTO records VALUES (?, ?, ?, ?); -- BatchExec



