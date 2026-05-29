import React, { useState, useEffect } from 'react';
import { api } from '../../services/api';
import './AdminStyles.css';

function Reports() {
  const [activeTab, setActiveTab] = useState(0);
  const [date, setDate] = useState('');
  const [masterId, setMasterId] = useState('');
  const [clientsReport, setClientsReport] = useState([]);
  const [earnings, setEarnings] = useState(null);
  const [popularService, setPopularService] = useState(null);
  const [genderRatio, setGenderRatio] = useState(null);
  const [permanentCount, setPermanentCount] = useState(null);
  const [busiestMaster, setBusiestMaster] = useState(null);
  const [masters, setMasters] = useState([]);

  useEffect(() => {
    api.masters.getAll()
      .then(data => setMasters(Array.isArray(data) ? data : []))
      .catch(err => console.error(err));
  }, []);

  const tabs = [
    'Клиенты по дате',
    'Заработок мастера',
    'Самая популярная услуга',
    'Соотношение полов',
    'Постоянные клиенты',
    'Самый загруженный мастер'
  ];

  const handleClientsByDate = async () => {
    if (!date) return alert('Выберите дату');
    try {
      const data = await api.admin.getReports.clientsByDate(date);
      setClientsReport(Array.isArray(data) ? data : []);
    } catch (err) {
      console.error(err);
      setClientsReport([]);
    }
  };

  const handleEarnings = async () => {
    if (!date || !masterId) return alert('Выберите дату и мастера');
    try {
      const data = await api.admin.getReports.masterEarnings(date, masterId);
      setEarnings(data);
    } catch (err) {
      console.error(err);
      setEarnings(null);
    }
  };

  const handlePopular = async () => {
    try {
      const data = await api.admin.getReports.mostPopularService();
      setPopularService(data);
    } catch (err) {
      console.error(err);
      setPopularService(null);
    }
  };

  const handleGenderRatio = async () => {
    try {
      const data = await api.admin.getReports.genderRatio();
      setGenderRatio(data);
    } catch (err) {
      console.error(err);
      setGenderRatio(null);
    }
  };

  const handlePermanent = async () => {
    if (!date) return alert('Выберите дату');
    try {
      const data = await api.admin.getReports.permanentClientsCount(date);
      setPermanentCount(data);
    } catch (err) {
      console.error(err);
      setPermanentCount(null);
    }
  };

  const handleBusiest = async () => {
    try {
      const data = await api.admin.getReports.busiestMaster();
      setBusiestMaster(data);
    } catch (err) {
      console.error(err);
      setBusiestMaster(null);
    }
  };

  const renderContent = () => {
    switch (activeTab) {
      case 0:
        return (
          <div className="report-card">
            <h3>Список клиентов на дату</h3>
            <div className="report-controls">
              <input type="date" value={date} onChange={e => setDate(e.target.value)} />
              <button onClick={handleClientsByDate}>Показать</button>
            </div>
            {clientsReport.length > 0 ? (
              <div className="report-result">
                {clientsReport.map((c, idx) => (
                  <div key={idx} className="client-item">
                    {c.name} - {c.service} - {c.date}
                  </div>
                ))}
              </div>
            ) : clientsReport.length === 0 && date && <p>Нет данных</p>}
          </div>
        );
      case 1:
        return (
          <div className="report-card">
            <h3>Заработок мастера</h3>
            <div className="report-controls">
              <input type="date" value={date} onChange={e => setDate(e.target.value)} />
              <select value={masterId} onChange={e => setMasterId(e.target.value)}>
                <option value="">Выберите мастера</option>
                {masters.map(m => <option key={m.id} value={m.id}>{m.name}</option>)}
              </select>
              <button onClick={handleEarnings}>Рассчитать</button>
            </div>
            {earnings && <div className="report-result">Заработок: {earnings.amount} ₽</div>}
          </div>
        );
      case 2:
        return (
          <div className="report-card">
            <h3>Самая распространённая услуга</h3>
            <div className="report-controls">
              <button onClick={handlePopular}>Показать</button>
            </div>
            {popularService && (
              <div className="report-result">
                {popularService.name} — {popularService.count} раз
              </div>
            )}
          </div>
        );
      case 3:
        return (
          <div className="report-card">
            <h3>Соотношение клиентов по полу</h3>
            <div className="report-controls">
              <button onClick={handleGenderRatio}>Показать</button>
            </div>
            {genderRatio && (
              <div className="report-result">
                Мужчины: {genderRatio.male}<br />
                Женщины: {genderRatio.female}
              </div>
            )}
          </div>
        );
      case 4:
        return (
          <div className="report-card">
            <h3>Постоянные клиенты на дату</h3>
            <div className="report-controls">
              <input type="date" value={date} onChange={e => setDate(e.target.value)} />
              <button onClick={handlePermanent}>Показать</button>
            </div>
            {permanentCount !== null && (
              <div className="report-result">Количество: {permanentCount.count}</div>
            )}
          </div>
        );
      case 5:
        return (
          <div className="report-card">
            <h3>Самый загруженный мастер</h3>
            <div className="report-controls">
              <button onClick={handleBusiest}>Показать</button>
            </div>
            {busiestMaster && (
              <div className="report-result">
                {busiestMaster.name} — {busiestMaster.total} клиентов
              </div>
            )}
          </div>
        );
      default:
        return null;
    }
  };

  return (
    <div className="material-admin">
      <h2>Отчёты</h2>
      <div className="tabs" style={{ display: 'flex', flexWrap: 'wrap', gap: '8px', marginBottom: '24px', borderBottom: '1px solid #ddd', paddingBottom: '8px' }}>
        {tabs.map((tab, idx) => (
          <button
            key={idx}
            className={`tab ${activeTab === idx ? 'active' : ''}`}
            onClick={() => setActiveTab(idx)}
            style={{ background: 'none', border: 'none', padding: '10px 20px', fontSize: '1rem', fontWeight: '500', borderRadius: '32px', cursor: 'pointer', transition: 'all 0.2s', color: activeTab === idx ? '#1e88e5' : '#5f6368', backgroundColor: activeTab === idx ? '#e3f2fd' : 'transparent' }}
          >
            {tab}
          </button>
        ))}
      </div>
      <div className="tab-content" style={{ background: 'white', borderRadius: '28px', padding: '24px', boxShadow: '0 2px 8px rgba(0,0,0,0.08)' }}>
        {renderContent()}
      </div>
    </div>
  );
}

export default Reports;