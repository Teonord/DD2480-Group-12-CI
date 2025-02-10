CREATE TABLE IF NOT EXISTS ci_builds (
    id SERIAL PRIMARY KEY,
    commit_id VARCHAR(40) NOT NULL,
    branch VARCHAR(255),
    author VARCHAR(255),
    commit_message TEXT,
    timestamp TIMESTAMP DEFAULT NOW(),
    compiled_successfully BOOLEAN,
    tests_passed BOOLEAN,
    execution_time INTERVAL,
    status VARCHAR(50), 
    build_log TEXT
);