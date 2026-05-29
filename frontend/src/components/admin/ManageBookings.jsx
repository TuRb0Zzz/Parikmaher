import React, { useState, useEffect } from 'react';
import { api } from '../../services/api';
import './AdminStyles.css';

function ManageBookings() {
  const [bookings, setBookings] = useState([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    loadBookings();
  }, []);

  const loadBookings = async () => {
    try {
      const data = await api.admin.getAllBookings();
      setBookings(Array.isArray(data) ? data : []);
    } catch (err) {
      console.error(err);
      setBookings([]);
    } finally {
      setLoading(false);
    }
  };

  const handleConfirm = async (id) => {
    if (window.confirm('Подтвердить запись?')) {
      try {
        await api.admin.confirmBooking(id);
        loadBookings();
      } catch (err) {
        alert('Ошибка подтверждения: ' + err.message);
      }
    }
  };

  if (loading) return <div>Загрузка...</div>;

  return (
    <div className="material-admin">
      <h2>Управление записями</h2>
      <div className="material-table-container">
        <table className="material-table">
          <thead>
            <tr>
              <th>ID</th>
              <th>Клиент</th>
              <th>Услуга</th>
              <th>Мастер</th>
              <th>Дата</th>
              <th>Статус</th>
              <th>Действия</th>
            </tr>
          </thead>
          <tbody>
            {bookings.map(b => (
              <tr key={b.id}>
                <td>{b.id}</td>
                <td>{b.user_name}</td>
                <td>{b.service_name}</td>
                <td>{b.master_name}</td>
                <td>{new Date(b.date).toLocaleString()}</td>
                <td>
                  {b.status === 'pending' && <span className="status-pending">Ожидает</span>}
                  {b.status === 'confirmed' && <span className="status-confirmed">Подтверждено</span>}
                  {b.status === 'cancelled' && <span className="status-cancelled">Отменено</span>}
                </td>
                <td>
                  {b.status === 'pending' && (
                    <button className="btn-primary" onClick={() => handleConfirm(b.id)}>Подтвердить</button>
                  )}
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
}

export default ManageBookings;