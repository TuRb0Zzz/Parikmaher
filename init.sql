CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    phone VARCHAR(20) NOT NULL,
    email VARCHAR(100) UNIQUE NOT NULL,
    password_hash VARCHAR(64) NOT NULL,
    role VARCHAR(20) DEFAULT 'user',
    category VARCHAR(20) DEFAULT 'regular',
    discount INT DEFAULT 0,
    photo_url VARCHAR(255) DEFAULT NULL
);

CREATE TABLE IF NOT EXISTS user_sessions (
    user_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    token VARCHAR(36) UNIQUE NOT NULL,
    expires_at TIMESTAMP NOT NULL,
    PRIMARY KEY (user_id)
);

CREATE TABLE IF NOT EXISTS services (
    id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    gender VARCHAR(20) NOT NULL,
    price INT NOT NULL,
    discount INT DEFAULT 0,
    photo_url VARCHAR(255) DEFAULT NULL
);

CREATE TABLE IF NOT EXISTS masters (
    id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    specialization VARCHAR(100) NOT NULL,
    rank VARCHAR(50) NOT NULL,
    photo_url VARCHAR(255) DEFAULT NULL
);

CREATE TABLE IF NOT EXISTS bookings (
    id SERIAL PRIMARY KEY,
    user_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    service_id INT NOT NULL REFERENCES services(id),
    master_id INT NOT NULL REFERENCES masters(id),
    booking_date TIMESTAMP NOT NULL,
    status VARCHAR(20) DEFAULT 'pending',
    created_at TIMESTAMP DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS clients (
    id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    category VARCHAR(50) NOT NULL,
    gender VARCHAR(20) NOT NULL,
    service VARCHAR(100) NOT NULL,
    date DATE NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);
CREATE INDEX IF NOT EXISTS idx_sessions_token ON user_sessions(token);
CREATE INDEX IF NOT EXISTS idx_bookings_user_id ON bookings(user_id);
CREATE INDEX IF NOT EXISTS idx_bookings_master_id ON bookings(master_id);
CREATE INDEX IF NOT EXISTS idx_bookings_service_id ON bookings(service_id);
CREATE INDEX IF NOT EXISTS idx_bookings_date ON bookings(booking_date);