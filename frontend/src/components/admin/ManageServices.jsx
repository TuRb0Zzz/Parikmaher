import React, { useState, useEffect } from 'react';
import { api } from '../../services/api';
import './AdminStyles.css';

function ManageServices() {
  const [services, setServices] = useState([]);
  const [form, setForm] = useState({ name: '', gender: 'male', price: '', discount: 0, photo: null });
  const [preview, setPreview] = useState(null);
  const [editingId, setEditingId] = useState(null);

  useEffect(() => {
    loadServices();
  }, []);

  const loadServices = async () => {
    try {
      const data = await api.admin.getServices();
      setServices(Array.isArray(data) ? data : []);
    } catch (err) {
      console.error(err);
    }
  };

  const handleFileChange = (e) => {
    const file = e.target.files[0];
    if (file) {
      setForm({ ...form, photo: file });
      setPreview(URL.createObjectURL(file));
    }
  };

  const handleSubmit = async (e) => {
    e.preventDefault();
    const fd = new FormData();
    fd.append('name', form.name);
    fd.append('gender', form.gender);
    fd.append('price', form.price);
    fd.append('discount', form.discount);
    if (form.photo) fd.append('photo', form.photo);
    try {
      if (editingId) {
        // Для редактирования фото не поддерживается
        await api.admin.updateService(editingId, {
          name: form.name,
          gender: form.gender,
          price: form.price,
          discount: form.discount,
        });
        setEditingId(null);
      } else {
        await api.admin.addService(fd);
      }
      setForm({ name: '', gender: 'male', price: '', discount: 0, photo: null });
      setPreview(null);
      loadServices();
    } catch (err) {
      alert('Ошибка: ' + err.message);
    }
  };

  const handleEdit = (service) => {
    setEditingId(service.id);
    setForm({
      name: service.name,
      gender: service.gender,
      price: service.price,
      discount: service.discount,
      photo: null,
    });
    setPreview(null);
  };

  const handleDelete = async (id) => {
    if (window.confirm('Удалить услугу?')) {
      await api.admin.deleteService(id);
      loadServices();
    }
  };

  return (
    <div className="material-admin">
      <h2>Управление услугами</h2>
      <form onSubmit={handleSubmit} className="material-form">
        <input
          placeholder="Название"
          value={form.name}
          onChange={e => setForm({...form, name: e.target.value})}
          required
        />
        <select
          value={form.gender}
          onChange={e => setForm({...form, gender: e.target.value})}
        >
          <option value="male">Мужской</option>
          <option value="female">Женский</option>
        </select>
        <input
          placeholder="Цена"
          type="number"
          value={form.price}
          onChange={e => setForm({...form, price: e.target.value})}
          required
        />
        <input
          placeholder="Скидка %"
          type="number"
          value={form.discount}
          onChange={e => setForm({...form, discount: e.target.value})}
        />
        <input type="file" accept="image/*" onChange={handleFileChange} />
        {preview && <img src={preview} alt="Preview" className="preview-image" />}
        <div className="form-actions">
          <button type="submit" className="btn-primary">{editingId ? 'Обновить' : 'Добавить'}</button>
          {editingId && (
            <button
              type="button"
              className="btn-secondary"
              onClick={() => {
                setEditingId(null);
                setForm({ name: '', gender: 'male', price: '', discount: 0, photo: null });
                setPreview(null);
              }}
            >
              Отмена
            </button>
          )}
        </div>
      </form>

      <div className="material-table-container">
        <table className="material-table">
          <thead>
            <tr>
              <th>ID</th><th>Название</th><th>Пол</th><th>Цена</th><th>Скидка %</th><th>Фото</th><th>Действия</th>
            </tr>
          </thead>
          <tbody>
            {services.map(s => (
              <tr key={s.id}>
                <td>{s.id}</td>
                <td>{s.name}</td>
                <td>{s.gender === 'male' ? 'Мужской' : 'Женский'}</td>
                <td>{s.price}</td>
                <td>{s.discount}</td>
                <td>{s.photo_url ? '✅' : '❌'}</td>
                <td>
                  <button className="btn-icon" onClick={() => handleEdit(s)}>✏️</button>
                  <button className="btn-icon" onClick={() => handleDelete(s.id)}>🗑️</button>
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
}

export default ManageServices;