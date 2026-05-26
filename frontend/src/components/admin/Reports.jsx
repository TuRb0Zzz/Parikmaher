import React, { useState } from 'react';
import { api } from '../../services/api';
import './Reports.css';

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

  React.useEffect(() => {
    api.masters.getAll().then(setMasters).catch(console.error);
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
    const data = await api.admin.getReports.clientsByDate(date);
    setClientsReport(data);
  };

  const handleEarnings = async () => {
    if (!date || !masterId) return alert('Выберите дату и мастера');
    const data = await api.admin.getReports.masterEarnings(date, masterId);
    setEarnings(data);
  };

  const handlePopular = async () => {
    const data = await api.admin.getReports.mostPopularService();
    setPopularService(data);
  };

  const handleGenderRatio = async () => {
    const data = await api.admin.getReports.genderRatio();
    setGenderRatio(data);
  };

  const handlePermanent = async () => {
    if (!date) return alert('Выберите дату');
    const data = await api.admin.getReports.permanentClientsCount(date);
    setPermanentCount(data);
  };

  const handleBusiest = async () => {
    const data = await api.admin.getReports.busiestMaster();
    setBusiestMaster(data);
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
                {clientsReport.map(c => (
                  <div key={c.id} className="client-item">
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
            <button onClick={handlePopular}>Показать</button>
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
            <button onClick={handleGenderRatio}>Показать</button>
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
            <button onClick={handleBusiest}>Показать</button>
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
    <div className="reports-container">
      <h2>Отчёты</h2>
      <div className="tabs">
        {tabs.map((tab, idx) => (
          <button
            key={idx}
            className={`tab ${activeTab === idx ? 'active' : ''}`}
            onClick={() => setActiveTab(idx)}
          >
            {tab}
          </button>
        ))}
      </div>
      <div className="tab-content">
        {renderContent()}
      </div>
    </div>
  );
}

export default Reports;