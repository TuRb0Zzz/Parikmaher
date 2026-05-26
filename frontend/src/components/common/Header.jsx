import React from 'react';
import { Link, useNavigate } from 'react-router-dom';
import { useAuth } from '../../contexts/AuthContext';
import './Header.css';

function Header() {
  const { user, logout } = useAuth();
  const navigate = useNavigate();

  const handleLogout = () => {
    logout();
    navigate('/');
  };

  return (
    <header className="header">
      <div className="header-container">
        <Link to="/" className="logo">Парикмахерская</Link>
        <nav className="nav">
          <Link to="/services" className="nav-link">Услуги</Link>
          <Link to="/masters" className="nav-link">Мастера</Link>
          {user ? (
            <>
              <Link to="/profile" className="nav-link">Личный кабинет</Link>
              {user.role === 'admin' && (
                <Link to="/admin" className="nav-link">Админ панель</Link>
              )}
              <button onClick={handleLogout} className="nav-button">Выйти</button>
            </>
          ) : (
            <Link to="/auth" className="nav-link">Войти / Регистрация</Link>
          )}
        </nav>
      </div>
    </header>
  );
}

export default Header;