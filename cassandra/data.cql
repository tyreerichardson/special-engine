-- Create a keyspace
CREATE KEYSPACE IF NOT EXISTS special_engine WITH REPLICATION = { 'class' : 'SimpleStrategy', 'replication_factor' : '1' };

-- Create a table for the messages (one-to-one)
CREATE TABLE IF NOT EXISTS special_engine.messages (
    message_id UUID,
    message_from TEXT,
    message_to TEXT,
    content TEXT,
    created_at TIMESTAMP,
    created_day DATE,
    PRIMARY KEY ((message_from, created_day), created_at)
) WITH CLUSTERING ORDER BY (created_at ASC);

-- Insert some data
INSERT INTO special_engine.messages
(message_id, message_from, message_to, content, created_at)
VALUES (uuid(), 'tyree', 'alice', 'Hey Alice!', toTimeStamp(now()));

INSERT INTO special_engine.messages
(message_id, message_from, message_to, content, created_at)
VALUES (uuid(), 'alice', 'tyree', 'Hey Tyree, How are you?', toTimeStamp(now()));

-- Create a table for the users
CREATE TABLE IF NOT EXISTS special_engine.users (
    user_id uuid PRIMARY KEY,
    username text,
    created_at timestamp,
    is_active boolean
);

-- EXAMPLE: Adjusting the table to have another user_id column
-- CREATE TABLE special_engine.msg (
--     user_id TEXT,
--     message_id UUID,
--     message_from TEXT,
--     message_to TEXT,
--     created_at TIMESTAMP,
--     PRIMARY KEY(user_id, created_at)
-- ) WITH CLUSTERING ORDER BY (created_at DESC);

-- 1. Primary Key with a Composite Partition Key:
-- A composite partition key uses multiple columns 
-- to define the partition key, allowing for more 
-- granular data distribution.
-- Code

-- NOTE: having the partition keys (sensor_id, reading_date) 
-- means that they are both required in the where clause

-- CREATE TABLE sensor_readings (
--    sensor_id TEXT,
--    reading_date DATE,
--    reading_time TIME,
--    temperature INT,
--    PRIMARY KEY ((sensor_id, reading_date), reading_time)
--);

-- Here, (sensor_id, reading_date) is the composite 
-- partition key. This means all readings 
-- for a specific sensor_id on a particular 
-- reading_date will be stored within the same 
-- partition. reading_time is a clustering column, 
-- which orders the data within that partition.

-- CREATE TABLE special_engine.msg (
--     message_id UUID,
--     message_from TEXT,
--     message_to TEXT, 
--     created_at TIMESTAMP, 
--     PRIMARY KEY((message_from, message_to), created_at) -- Not going to make two primary keys this is not necessary
-- ) WITH CLUSTERING ORDER BY (created_at DESC);

-- EXAMPLE QUERY:
-- CREATE MATERIALIZED VIEW users_by_email AS
-- SELECT * FROM users
-- WHERE email IS NOT NULL AND user_id IS NOT NULL
-- PRIMARY KEY (email, user_id);
