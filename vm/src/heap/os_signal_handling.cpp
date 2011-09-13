/******************************************************************************
 *  Copyright (c) 2008 - 2011 IBM Corporation and others.
 *  All rights reserved. This program and the accompanying materials
 *  are made available under the terms of the Eclipse Public License v1.0
 *  which accompanies this distribution, and is available at
 *  http://www.eclipse.org/legal/epl-v10.html
 * 
 *  Contributors:
 *    Mattias De Wael, Vrije Universiteit Brussel - Parallel Garbage Collection
 *    Wouter Amerijckx, Vrije Universiteit Brussel - Parallel Garbage Collection
 ******************************************************************************/


#include "headers.h"

void signal_handler_setEax(int sig, siginfo_t *info, ucontext_t *uap){
  printf("In signal handler...\n");
  //uap->uc_mcontext->__ss.__eip+=6; // write
  uap->uc_mcontext->__ss.__eip+=7; //read
  uap->uc_mcontext->__ss.__eax=TRAPPED;
}

int install_signalhandler(int signum,  void* sig_handler){
  struct sigaction sa;
  
  sa.sa_handler = (void(*)(int))sig_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO | SA_NODEFER;    
  
  return sigaction(signum, &sa, NULL);
}

int install_signalhandler_protectedPageAcces(){
  install_signalhandler(SIGSEGV, (void*)signal_handler_setEax);
  install_signalhandler(SIGBUS, (void*)signal_handler_setEax);
}

void TEST_force_protectedPage_signal_trap(){
  return;
  install_signalhandler_protectedPageAcces();
  Object* v_ptr = (Object*)mmap(NULL,20*sizeof(int),PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON , NULL, 0); //OK
  //int* v_ptr = (int*)malloc(20*sizeof(int)); //NOT ok: not aligned?
  //int* v_ptr = (int*)valloc(20*sizeof(int)); //OK
  
  if(v_ptr == MAP_FAILED){
    printf("MMAP failed(%s)\n",strerror(errno));
    if( errno == EINVAL) printf("EINVAL\n");
  } else {
    printf("Pointer to int:%p\n",v_ptr);
    printf("page size = %d\n",getpagesize());
    
    int prot_res = 1;//mprotect(v_ptr,10*sizeof(int),PROT_NONE);
    if(prot_res < 0){
      if( errno == EACCES) printf("EACCES\n");
      if( errno == EINVAL) printf("EINVAL\n");
      if( errno == ENOTSUP) printf("ENOTSUP\n");
    } else {
    
    printf("Protection result:%d\n",prot_res);
    
    int r_eax;
    *((int*)v_ptr) = 4; // write
    asm volatile ("movl %%eax, %0\n" :"=r"(r_eax));
    if(r_eax == TRAPPED){
      printf("TRAPPED, returning 1\n");
      printf("TRAPPED, returning 2\n");
    } else {
      printf("NOT TRAPPED, returning 0\n");
    }
      prot_res = mprotect(v_ptr,10*sizeof(int),PROT_READ | PROT_WRITE);
    }
  }
}