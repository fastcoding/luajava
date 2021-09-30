#
# Makefile for LuaJava Linux Distribution
#
SYSNAME=$(shell uname)
include ./config-$(SYSNAME)

CLASSES     = \
	classes/java/org/keplerproject/luajava/CPtr.class \
	classes/java/org/keplerproject/luajava/JavaFunction.class \
	classes/java/org/keplerproject/luajava/LuaException.class \
	classes/java/org/keplerproject/luajava/LuaInvocationHandler.class \
	classes/java/org/keplerproject/luajava/LuaJavaAPI.class \
	classes/java/org/keplerproject/luajava/LuaObject.class \
	classes/java/org/keplerproject/luajava/LuaState.class \
	classes/java/org/keplerproject/luajava/LuaStateFactory.class \
	classes/java/org/keplerproject/luajava/Console.class

DOC_CLASSES	= \
	src/java/org/keplerproject/luajava/JavaFunction.java \
	src/java/org/keplerproject/luajava/LuaException.java \
	src/java/org/keplerproject/luajava/LuaInvocationHandler.java \
	src/java/org/keplerproject/luajava/LuaObject.java \
	src/java/org/keplerproject/luajava/LuaState.java \
	src/java/org/keplerproject/luajava/LuaStateFactory.java \
	src/java/org/keplerproject/luajava/Console.java

OBJDIR      = classes
OBJS        = $(OBJDIR)/luajava.o $(OBJDIR)/javavm.o
.SUFFIXES: .java .class

#
# Targets
#
run: build
	@echo ------------------
	@echo Build Complete
	@echo ------------------

build: $(JAR_FILE) $(SO_FILE)

#
# Build .class files.
#
classes/java/org/keplerproject/luajava/%.class:src/java/org/keplerproject/luajava/%.java
	@echo $(JDK)/bin/javac -d classes -sourcepath ./src/java $<
	$(JDK)/bin/javac -d classes -sourcepath ./src/java $<

#
# Create the JAR
#
$(JAR_FILE): $(CLASSES)
	cd classes; \
	$(JDK)/bin/jar cvf ../$(JAR_FILE) org/keplerproject/luajava/*.class; \
	cd ..;
  
#
# Create the API Documentation
#
apidoc:
	$(JDK)/bin/javadoc -public -classpath classes -quiet -d "doc/us/API" $(DOC_CLASSES)

#
# Build .c files.
#
$(SO_FILE): $(OBJS)
	#export MACOSX_DEPLOYMENT_TARGET=10.3; $(CC) $(LIB_OPTION) -o $@ $? $(LIB_LUA)
	rm -rf $@
	$(CC) $(LIB_OPTION) -o $@ $? $(LIB_LUA)

src/c/luajava.c: src/c/luajava.h

src/c/luajava.h:
	$(JDK)/bin/javac -h src/c -classpath "$(JAR_FILE)" ./src/java/org/keplerproject/luajava/LuaState.java
	mv src/c/org_keplerproject_luajava_LuaState.h src/c/luajava.h
  

## regras implicitas para compilacao

$(OBJDIR)/%.o:  src/c/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# Check that the user has a valid JDK install.  This will cause a
# premature death if JDK is not defined.
#

#checkjdk: $(JDK)/bin/java

#
# Cleanliness.
#
clean:
	rm -f $(JAR_FILE)
	rm -f $(SO_FILE)
	rm -rf $(OBJS)
	rm -rf doc/us/API
	rm -f src/java/org/keplerproject/luajava/*.class src/c/*.o src/c/luajava.h
	rm -f $(TAR_FILE) $(ZIP_FILE)

dist:	dist_dir
	tar -czf $(TAR_FILE) --exclude \*CVS\* --exclude *.class --exclude *.o --exclude *.h --exclude $(TAR_FILE) --exclude $(ZIP_FILE) $(DIST_DIR)
	zip -qr $(ZIP_FILE) $(DIST_DIR)/* -x ./\*CVS\* *.class *.o *.h ./$(TAR_FILE) ./$(ZIP_FILE)

dist_dir:	apidoc
	mkdir -p $(DIST_DIR)
	cp -R src $(DIST_DIR)
	cp -R doc $(DIST_DIR)
	cp -R test $(DIST_DIR)
	cp config License.txt Makefile config.win Makefile.win $(DIST_DIR)

