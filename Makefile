CXX = clang++
CXX_FLAGS =

SRCDIR = src
OUTDIR = out
OBJDIR = $(OUTDIR)/obj
BINDIR = $(OUTDIR)/bin
PACKAGES = "opencv libglog"

INCLUDE = -I$(SRCDIR) $(shell pkg-config $(PACKAGES) --cflags-only-I)
LDFLAGS = $(shell pkg-config $(PACKAGES) --libs)


CC_FILES = $(shell find $(SRCDIR) -name "*.cc")
OBJ_FILES = $(addprefix $(OBJDIR)/, $(patsubst %.cc, %.o, $(CC_FILES)))

$(BINDIR)/splat: $(OBJ_FILES)
	@if [ ! -d $(dir $@) ]; then \
		echo "MKDIR $(dir $@)"; mkdir -p $(dir $@); \
	fi
	@echo "LINK $@"
	@$(CXX) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: %.cc
	@if [ ! -d $(dir $@) ]; then \
		echo "MKDIR $(dir $@)"; mkdir -p $(dir $@); \
	fi
	@echo "CXX $@"
	@$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ -c $<

clean:
	@rm -fr $(OUTDIR)
