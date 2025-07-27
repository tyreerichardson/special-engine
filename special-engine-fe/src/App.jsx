
import List from './components/list/list'
import Chat from './components/chat/chat'
import Detail from './components/detail/detail'
import { useState } from 'react'
import sendIcon from './assets/send.svg'
import './App.css'


function App() {
  return (
    <>
    {/* The container of the messages */}
    <div className='container'>
      <List/>
      <Chat/>
      <Detail/>
    </div>
    
    </>
  )
}

export default App
 