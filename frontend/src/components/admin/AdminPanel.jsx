import React from 'react';
import { Link, Outlet } from 'react-router-dom';
import './AdminPanel.css';

function AdminPanel() {
  return (
    <div className="admin-panel">
      <div className="admin-sidebar">
        <ul>
          <li><Link to="/admin/clients">Клиенты</Link></li>
          <li><Link to="/admin/masters">Мастера</Link></li>
          <li><Link to="/admin/services">Услуги</Link></li>
          <li><Link to="/admin/bookings">Записи</Link></li>
          <li><Link to="/admin/reports">Отчёты</Link></li>
        </ul>
      </div>
      <div className="admin-content">
        <Outlet />
      </div>
    </div>
  );
}

export default AdminPanel;