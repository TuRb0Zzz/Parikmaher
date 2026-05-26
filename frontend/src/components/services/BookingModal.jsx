import React, { useState, useEffect } from 'react';
import { api } from '../../services/api';
import './BookingModal.css';

function BookingModal({ service, onClose, onSuccess }) {
  const [masters, setMasters] = useState([]);
  const [selectedMaster, setSelectedMaster] = useState('');
  const [date, setDate] = useState('');
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState('');

  useEffect(() => {
    api.masters.getAll().then(data => {
      const filtered = data.filter(m => m.specialization === service.gender);
      setMasters(filtered);
    }).catch(console.error);
  }, [service]);

  const handleSubmit = async (e) => {
    e.preventDefault();
    if (!selectedMaster || !date) {
      setError('Заполните все поля');
      return;
    }
    setLoading(true);
    try {
      await api.services.book(service.id, selectedMaster, date);
      onSuccess();
      onClose();
    } catch (err) {
      setError(err.message);
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="modal-overlay" onClick={onClose}>
      <div className="modal-content" onClick={e => e.stopPropagation()}>
        <button className="modal-close" onClick={onClose}>×</button>
        <h2>Запись на услугу</h2>
        <p className="modal-service-name">{service.name} — {service.price} ₽</p>
        <form onSubmit={handleSubmit}>
          <div className="form-group">
            <label>Выберите мастера</label>
            <select value={selectedMaster} onChange={e => setSelectedMaster(e.target.value)} required>
              <option value="">Мастер</option>
              {masters.map(m => (
                <option key={m.id} value={m.id}>{m.name} ({m.specialization === 'male' ? 'муж.' : 'жен.'})</option>
              ))}
            </select>
          </div>
          <div className="form-group">
            <label>Дата и время</label>
            <input type="datetime-local" value={date} onChange={e => setDate(e.target.value)} required />
          </div>
          {error && <div className="error-message">{error}</div>}
          <button type="submit" className="submit-btn" disabled={loading}>
            {loading ? 'Запись...' : 'Записаться'}
          </button>
        </form>
      </div>
    </div>
  );
}

export default BookingModal;