-- Create a keyspace
CREATE KEYSPACE IF NOT EXISTS special_engine WITH REPLICATION = { 'class' : 'SimpleStrategy', 'replication_factor' : '1' };

-- Create a table for the messages (one-to-one)
CREATE TABLE IF NOT EXISTS special_engine.messages (
message_id uuid PRIMARY KEY,
message_from text,
message_to text,
content text,
created_at timestamp
);

-- Insert some data
INSERT INTO special_engine.messages
(message_id, message_from, message_to, content, created_at)
VALUES (uuid(), 'tyree', 'alice', 'Hey Alice!', toTimeStamp(now()));

INSERT INTO special_engine.messages
(message_id, message_from, message_to, content, created_at)
VALUES (uuid(), 'alice', 'tyree', 'Hey Tyree, How are you?', toTimeStamp(now()));

