import React, { useState, useEffect } from 'react';
import { useAuth } from '../../contexts/AuthContext';
import { api } from '../../services/api';
import './Profile.css';

function Profile() {
  const { user } = useAuth();
  const [bookings, setBookings] = useState([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    if (user) {
      api.bookings.getUserBookings()
        .then(data => {
          setBookings(Array.isArray(data) ? data : []);
        })
        .catch(err => {
          console.error('Ошибка загрузки записей:', err);
          setBookings([]);
        })
        .finally(() => setLoading(false));
    } else {
      setBookings([]);
      setLoading(false);
    }
  }, [user]);

  const handleCancel = async (bookingId) => {
    if (window.confirm('Отменить запись?')) {
      try {
        await api.bookings.cancel(bookingId);
        setBookings(prev => prev.filter(b => b.id !== bookingId));
      } catch (err) {
        alert('Ошибка отмены: ' + err.message);
      }
    }
  };

  if (loading) return <div className="loading">Загрузка...</div>;

  return (
    <div className="profile-container">
      <div className="profile-header">
        <div className="profile-avatar">
          <img 
            src={user?.photo_url || '/default-avatar.png'} 
            alt="Аватар" 
          />
        </div>
        <div className="profile-info">
          <h2>Личный кабинет</h2>
          <p><strong>Имя:</strong> {user?.name}</p>
          <p><strong>Email:</strong> {user?.email}</p>
          <p><strong>Телефон:</strong> {user?.phone}</p>
          <p><strong>Категория:</strong> {user?.category === 'permanent' ? 'Постоянный клиент' : 'Обычный'}</p>
          {user?.category === 'permanent' && <p><strong>Скидка:</strong> {user?.discount}%</p>}
        </div>
      </div>
      <div className="bookings-history">
        <h3>Мои записи</h3>
        {bookings.length === 0 ? (
          <p>У вас пока нет записей</p>
        ) : (
          <ul>
            {bookings.map(booking => (
              <li key={booking.id}>
                <span>{booking.service_name} - {booking.master_name} - {new Date(booking.date).toLocaleString()}</span>
                <button onClick={() => handleCancel(booking.id)}>Отменить</button>
              </li>
            ))}
          </ul>
        )}
      </div>
    </div>
  );
}

export default Profile;