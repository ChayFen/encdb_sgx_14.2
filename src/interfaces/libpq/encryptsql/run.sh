#!/bin/bash
rm -rf   /etc/encryptsql/*.log
base_path=/media/zheng/68DC6A68DC6A310C/container_path/opegauss_exp/mysql0 
func() {
    # rm -rf /tmp/encryptsql/*.lock 
    # echo "running for create"
    # rm -rf $base_path/$1/create
    # mkdir -p $base_path/$1/create
    # build/bin/sqltest 100000 1 1 0 0 0 0 | tee  $base_path/$1/create/run.log
    # echo "create done"
    # echo "running for insert"
    # rm -rf $base_path/$1/insert
    # mkdir -p $base_path/$1/insert
    # build/bin/sqltest 100000 1 0 1 0 0 0 | tee  $base_path/$1/insert/run.log
    # cp /etc/encryptsql/*.log $base_path/$1/insert
    # rm -rf  /etc/encryptsql/counter_* /etc/encryptsql/*.log 
    # echo "insert done"
    for ((i=1; i<=64; i=$i*2))
    do
        echo "running for $i"
        rm -rf $base_path/$1/th$i
        mkdir -p $base_path/$1/th$i
        # sleep_t=$[ ($i+1)*3 ]
        # if [ $i -ge 16 ]
        # then
        #     sleep_t=$[ $i*1 ]
        # fi
        sleep_t=1
        for ((j=0; j<=$sleep_t; j=$j+10))
        do    
            echo "sleeping1 ... $j"
            sleep 10
        done	
        build/bin/sqltest 1 $i 0 0 1 0 0 $base_path/$1/th$i/run.log
        if [ $? != 0 ]
        then
            echo $?
            exit 1  #参数正确，退出状态为0
            sleep 30
        fi
        echo "finished for thread=$i with $?"
        cp /etc/encryptsql/*.log $base_path/$1/th$i/     
        rm -rf  /etc/encryptsql/counter_* /etc/encryptsql/*.log 
        for ((j=0; j<=$sleep_t/2; j=$j+10))
        do        
            echo "sleeping2 ... $j"
                sleep 10
        done
    done
    # echo "running for update"
    # rm -rf $base_path/$1/update
    # mkdir -p $base_path/$1/update
    # build/bin/sqltest 1 1 0 0 0 1 0 | tee  $base_path/$1/update/run.log
    # cp /etc/encryptsql/*.log $base_path/$1/insert
    # rm -rf  /etc/encryptsql/counter_* /etc/encryptsql/*.log 
    # echo "update done"
}

#func hwore postgres
func raw postgres


