import React from 'react';
import { BrowserRouter, Routes, Route, Navigate } from 'react-router-dom';
import { AuthProvider, useAuth } from './contexts/AuthContext';
import Header from './components/common/Header';
import Auth from './components/auth/Auth';
import ServicesPage from './components/services/ServicesPage';
import MastersPage from './components/masters/MastersPage';
import Profile from './components/profile/Profile';
import AdminPanel from './components/admin/AdminPanel';
import ManageClients from './components/admin/ManageClients';
import ManageMasters from './components/admin/ManageMasters';
import ManageServices from './components/admin/ManageServices';
import Reports from './components/admin/Reports';
import HomePage from './components/home/HomePage';
import './App.css';

function ProtectedRoute({ children, allowedRoles }) {
  const { user, loading } = useAuth();
  if (loading) return <div className="loading">Загрузка...</div>;
  if (!user) return <Navigate to="/auth" />;
  if (allowedRoles && !allowedRoles.includes(user.role)) return <Navigate to="/" />;
  return children;
}

function AppRoutes() {
  return (
    <div className="app">
      <Header />
      <div className="main-content">
        <Routes>
          <Route path="/" element={<HomePage />} />
          <Route path="/services" element={<ServicesPage />} />
          <Route path="/masters" element={<MastersPage />} />
          <Route path="/auth" element={<Auth />} />
          <Route path="/profile" element={<ProtectedRoute><Profile /></ProtectedRoute>} />
          <Route path="/admin" element={<ProtectedRoute allowedRoles={['admin']}><AdminPanel /></ProtectedRoute>}>
            <Route path="clients" element={<ManageClients />} />
            <Route path="masters" element={<ManageMasters />} />
            <Route path="services" element={<ManageServices />} />
            <Route path="reports" element={<Reports />} />
          </Route>
        </Routes>
      </div>
    </div>
  );
}

function App() {
  return (
    <BrowserRouter>
      <AuthProvider>
        <AppRoutes />
      </AuthProvider>
    </BrowserRouter>
  );
}

export default App;