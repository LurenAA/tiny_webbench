
阅读webbench之后的练习模仿

在主进程中只能通过循环fscanf来读取管道，如果wait后来读会读到的数据全是一样的，
因为所有子进程共享一个FILE，而FILE是系统级别的