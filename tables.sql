-- server side

CREATE TABLE accounts (
    id: NVARCHAR(36) NOT NULL,
    pswd: NVARCHAR(256) NOT NULL,
    PRIMARY KEY (id)
);

CREATE TABLE memos(
    id: NVARCHAR(36) NOT NULL REFERENCES accounts,
    group: NVARCHAR(256) NOT NULL,
    title: NVARCHAR(128) NOT NULL,
    memo: NVARCHAR(1024) NOT NULL,
    create_time: TIME NOT NULL,
    modify_time: TIME,
    PRIMARY KEY(id, group, title)
);

-- accounts
-- LOG_IN
SELECT COUNT(*) FROM accounts WHERE id = ? AND pswd = ?;
-- LOG_OUT
-- SELECT COUNT(*) FROM accounts WHERE id = ?;
-- CREATE_ACCOUNT,
INSERT INTO accounts(id, pswd) VALUES (?, ?);

