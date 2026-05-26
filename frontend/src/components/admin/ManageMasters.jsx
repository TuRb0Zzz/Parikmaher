import React, { useState, useEffect } from 'react';
import { api } from '../../services/api';
import './AdminStyles.css';

function ManageMasters() {
  const [masters, setMasters] = useState([]);
  const [form, setForm] = useState({ name: '', specialization: 'male', rank: '', photo: null });
  const [preview, setPreview] = useState(null);
  const [editingId, setEditingId] = useState(null);

  useEffect(() => {
    loadMasters();
  }, []);

  const loadMasters = async () => {
    try {
      const data = await api.admin.getMasters();
      setMasters(Array.isArray(data) ? data : []);
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
    fd.append('specialization', form.specialization);
    fd.append('rank', form.rank);
    if (form.photo) fd.append('photo', form.photo);
    try {
      if (editingId) {
        // Для редактирования фото не поддерживается (можно расширить API)
        await api.admin.updateMaster(editingId, { name: form.name, specialization: form.specialization, rank: form.rank });
        setEditingId(null);
      } else {
        await api.admin.addMaster(fd);
      }
      setForm({ name: '', specialization: 'male', rank: '', photo: null });
      setPreview(null);
      loadMasters();
    } catch (err) {
      alert('Ошибка: ' + err.message);
    }
  };

  const handleEdit = (master) => {
    setEditingId(master.id);
    setForm({
      name: master.name,
      specialization: master.specialization,
      rank: master.rank,
      photo: null,
    });
    setPreview(null);
  };

  const handleDelete = async (id) => {
    if (window.confirm('Удалить мастера?')) {
      await api.admin.deleteMaster(id);
      loadMasters();
    }
  };

  return (
    <div className="material-admin">
      <h2>Управление мастерами</h2>
      <form onSubmit={handleSubmit} className="material-form">
        <input
          placeholder="ФИО"
          value={form.name}
          onChange={e => setForm({...form, name: e.target.value})}
          required
        />
        <select
          value={form.specialization}
          onChange={e => setForm({...form, specialization: e.target.value})}
        >
          <option value="male">Мужской зал</option>
          <option value="female">Женский зал</option>
        </select>
        <input
          placeholder="Разряд"
          value={form.rank}
          onChange={e => setForm({...form, rank: e.target.value})}
          required
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
                setForm({ name: '', specialization: 'male', rank: '', photo: null });
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
              <th>ID</th><th>ФИО</th><th>Специализация</th><th>Разряд</th><th>Фото</th><th>Действия</th>
            </tr>
          </thead>
          <tbody>
            {masters.map(m => (
              <tr key={m.id}>
                <td>{m.id}</td>
                <td>{m.name}</td>
                <td>{m.specialization === 'male' ? 'Мужской' : 'Женский'}</td>
                <td>{m.rank}</td>
                <td>{m.photo_url ? '✅' : '❌'}</td>
                <td>
                  <button className="btn-icon" onClick={() => handleEdit(m)}>✏️</button>
                  <button className="btn-icon" onClick={() => handleDelete(m.id)}>🗑️</button>
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
}

export default ManageMasters;