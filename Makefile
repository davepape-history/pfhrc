CFLAGS = -I/usr/local/CAVE/include -O -UIRISGL -DOPENGL
C++FLAGS = $(CFLAGS)

OBJS = softModel.o softMesh.o softFace.o softMaterial.o softTexture.o \
	softPatch.o softSpline.o tokenStream.o

dso: load_hrc.o $(OBJS)
	ld -shared -all -check_registry /usr/lib/so_locations \
		-set_version sgi3.0 \
		load_hrc.o $(OBJS) \
		-o libpfhrc_ogl.so.3

dso2: load_hrc.o $(OBJS)
	ld -shared -all -check_registry /usr/lib/so_locations \
		-set_version sgi2.0 \
		load_hrc.o $(OBJS) \
		-o libpfhrc_ogl.so.2
