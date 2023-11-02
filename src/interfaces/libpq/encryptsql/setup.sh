func_file_exist_check() {
    if [ ! -e $1 ]; then
        echo "file $1 is not exist, please check if the path where this script is correct."
        exit -1
    fi
}

func_modify_makefile() {
    func_file_exist_check $1
    if [ $2 == 1 ]
    then
        # pwd1=$(pwd)
        # echo "${pwd1}"
#         echo "Modifying libpq Makefile..."
#         sed -i 's/SHLIB_EXPORTS = exports.txt/SHLIB_EXPORTS = exports.txt\n \
# SUBDIRS = encryptsql 	\
# .NOTPARALLEL: 			\
# $(recurse)				\
# SHLIB_LINK += -Lencryptsql/build/lib -lencryptsql -lssl/g' $1
    rm -f ../Makefile
    cp replaced_file/Makefile.libpq ../Makefile
    fi

    if [ $2 == 2 ]
    then
        echo "Modifying psql Makefile..."
        # sed -i 's/$(OBJS) $(LDFLAGS)/$(OBJS) -L$(top_builddir)/src/interfaces/libpq/encryptsql/build/lib -lencryptsql $(LDFLAGS)/g'  $1
        rm -f $2
        cp replaced_file/Makefile.psql $2
    fi
}

func_modify_psql_file() {
    # sed -i '1188i disableAsNeeded(); if (encryptOneSql) query = encryptOneSql(query);'  $1
    # sed -i '52i extern const char *encryptOneSql(const char *sql) __attribute__((weak));' $1

    cp replaced_file/fe-exec.9.6.24.c ../
    rm  ../fe-exec.c
    mv ../fe-exec.9.6.24.c ../fe-exec.c

}


func_modify_makefile "../Makefile" 1
func_modify_makefile "../../../bin/psql/Makefile" 2

func_modify_psql_file 
sudo mkdir -p /etc/encryptsql/enclave && sudo cp config.json /etc/encryptsql/
ln -s ../../../include db_include
# export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:$(dirname $0)
