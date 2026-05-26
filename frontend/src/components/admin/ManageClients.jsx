import React, { useState, useEffect } from 'react';
import { api } from '../../services/api';
import './ManageClients.css';

function ManageClients() {
  const [clients, setClients] = useState([]);
  const [form, setForm] = useState({ name: '', category: 'regular', gender: 'male', service: '', date: '' });
  const [editingId, setEditingId] = useState(null);

  useEffect(() => {
    loadClients();
  }, []);

  const loadClients = async () => {
    try {
      const data = await api.admin.getClients();
      setClients(Array.isArray(data) ? data : []);
    } catch (err) {
      console.error(err);
    }
  };

  const handleSubmit = async (e) => {
    e.preventDefault();
    try {
      if (editingId) {
        await api.admin.updateClient(editingId, form);
        setEditingId(null);
      } else {
        await api.admin.addClient(form);
      }
      setForm({ name: '', category: 'regular', gender: 'male', service: '', date: '' });
      loadClients();
    } catch (err) {
      alert('Ошибка: ' + err.message);
    }
  };

  const handleEdit = (client) => {
    setEditingId(client.id);
    setForm({
      name: client.name,
      category: client.category,
      gender: client.gender,
      service: client.service,
      date: client.date,
    });
  };

  const handleDelete = async (id) => {
    if (window.confirm('Удалить клиента?')) {
      await api.admin.deleteClient(id);
      loadClients();
    }
  };

  return (
    <div className="material-admin">
      <h2>Управление клиентами</h2>
      <form onSubmit={handleSubmit} className="material-form">
        <input placeholder="Имя" value={form.name} onChange={e => setForm({...form, name: e.target.value})} required />
        <select value={form.category} onChange={e => setForm({...form, category: e.target.value})}>
          <option value="regular">Обычный</option>
          <option value="permanent">Постоянный</option>
        </select>
        <select value={form.gender} onChange={e => setForm({...form, gender: e.target.value})}>
          <option value="male">Мужской</option>
          <option value="female">Женский</option>
        </select>
        <input placeholder="Услуга" value={form.service} onChange={e => setForm({...form, service: e.target.value})} required />
        <input type="date" value={form.date} onChange={e => setForm({...form, date: e.target.value})} required />
        <div className="form-actions">
          <button type="submit" className="btn-primary">{editingId ? 'Обновить' : 'Добавить'}</button>
          {editingId && <button type="button" className="btn-secondary" onClick={() => { setEditingId(null); setForm({ name: '', category: 'regular', gender: 'male', service: '', date: '' }); }}>Отмена</button>}
        </div>
      </form>

      <div className="material-table-container">
        <table className="material-table">
          <thead>
            <tr><th>ID</th><th>Имя</th><th>Категория</th><th>Пол</th><th>Услуга</th><th>Дата</th><th>Действия</th></tr>
          </thead>
          <tbody>
            {clients.map(c => (
              <tr key={c.id}>
                <td>{c.id}</td>
                <td>{c.name}</td>
                <td>{c.category === 'permanent' ? 'Постоянный' : 'Обычный'}</td>
                <td>{c.gender === 'male' ? 'Мужской' : 'Женский'}</td>
                <td>{c.service}</td>
                <td>{c.date}</td>
                <td>
                  <button className="btn-icon" onClick={() => handleEdit(c)}>✏️</button>
                  <button className="btn-icon" onClick={() => handleDelete(c.id)}>🗑️</button>
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
}

export default ManageClients;