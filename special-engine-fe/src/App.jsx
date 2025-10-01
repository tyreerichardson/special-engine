
import List from './components/list/list'
import Chat from './components/chat/chat'
import Detail from './components/detail/detail'
import { useState } from 'react'
import sendIcon from './assets/send.svg'
import './App.css'
import User from './models/user';
import ClientSocket from './services/client.jsx';

function App() {
  return (
    <>
    {/* The container of the messages */}
    <User/>
	<ClientSocket/>
    <div className='container'>
      <List/>
      <Chat/>
      <Detail/>
    </div>
    
    </>
  )
}

export default App
 
