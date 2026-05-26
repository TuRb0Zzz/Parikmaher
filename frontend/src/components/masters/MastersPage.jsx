import React, { useState, useEffect } from 'react';
import { api } from '../../services/api';
import './MastersPage.css';

function MastersPage() {
  const [masters, setMasters] = useState([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    api.masters.getAll()
      .then(data => setMasters(Array.isArray(data) ? data : []))
      .catch(err => {
        console.error('Ошибка загрузки мастеров:', err);
        setMasters([]);
      })
      .finally(() => setLoading(false));
  }, []);

  if (loading) return <div className="loading">Загрузка...</div>;

  return (
    <div className="masters-page">
      <h1 className="page-title">Наши мастера</h1>
      <div className="masters-grid">
        {masters.map(master => (
          <div key={master.id} className="master-card">
            <img 
              src={master.photo_url || '/placeholder-master.png'} 
              alt={master.name} 
              className="master-photo" 
            />
            <h3 className="master-name">{master.name}</h3>
            <p className="master-specialization">
              {master.specialization === 'male' ? 'Мужской зал' : 'Женский зал'}
            </p>
            <p className="master-rank">Разряд: {master.rank}</p>
          </div>
        ))}
      </div>
    </div>
  );
}

export default MastersPage;