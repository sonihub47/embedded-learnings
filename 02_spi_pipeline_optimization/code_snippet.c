// optimize the latency
// T1 task
len = read_fifo_length();

while (len)
{
    spiLen = len > COL_SIZE ? COL_SIZE : len;
    len -= spiLen;
    spiTransaction.count = spiLen;
    spiTransaction.txBuf = txBuffer;
    spiTransaction.rxBuf = rxBuffer;
    SPI_transfer(spi, &spiTransaction);
    mptr = osMailAlloc(mail_A, osWaitForever);       // Allocate memory
    memcpy(mptr->rxBuffer, rxBuffer, COL_SIZE);
    osMailPut(mail_A, mptr); 
}

// T3
while(1)
{
    evt = osMailGet(mail_A, osWaitForever);        // wait for mail
    if (evt.status == osEventMail) {
        rptr = evt.value.p;
        mptr = osMailAlloc(mail_B, osWaitForever);       // Allocate memory
        image_conversion(rptr->rxBuffer, NUM_COL, mptr->rgb565);
        osMailPut(mail_B, mptr);
        osMailFree(mail_A, rptr);                    // free memory allocated for mail
    }    
}

// T2
while(1)
{
    evt = osMailGet(mail_B, osWaitForever);        // wait for mail
    if (evt.status == osEventMail) {
        rptr = evt.value.p;
        image_processing(rptr->rgb565);
        osMailFree(mail_B, rptr);                    // free memory allocated for mail
    } 
}


