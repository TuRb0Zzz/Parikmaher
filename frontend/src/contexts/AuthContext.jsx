import React, { createContext, useState, useEffect, useContext } from 'react';
import { api } from '../services/api';

const AuthContext = createContext();

export const AuthProvider = ({ children }) => {
  const [user, setUser] = useState(null);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    const loadUser = async () => {
      try {
        const userData = await api.auth.me();   // теперь me тоже возвращает user напрямую
        setUser(userData);
      } catch (err) {
        console.error('Ошибка загрузки пользователя:', err);
        setUser(null);
      } finally {
        setLoading(false);
      }
    };
    loadUser();
  }, []);

  const login = async (email, password) => {
    const userData = await api.auth.login(email, password);
    setUser(userData);   // userData — это уже объект пользователя
    return userData;
  };

  const register = async (userData) => {
    const newUser = await api.auth.register(userData);
    setUser(newUser);
    return newUser;
  };

  const logout = async () => {
    try {
      await api.auth.logout();
    } catch (e) {
      console.error('Logout error', e);
    } finally {
      setUser(null);
    }
  };

  return (
    <AuthContext.Provider value={{ user, loading, login, register, logout }}>
      {children}
    </AuthContext.Provider>
  );
};

export const useAuth = () => useContext(AuthContext);