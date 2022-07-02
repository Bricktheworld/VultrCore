CXX := clang++
INCLUDES := -Isrc -Ivendor
CXXFLAGS := -g -DDEBUG -std=c++20 -fPIC -Wno-reorder -Wno-deprecated-enum-enum-conversion -Wno-nullability-completeness -Wno-deprecated-volatile
# Intentionally not evaluated, do NOT convert = to :=
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEP)/$*.d

ENGINE := vultr
EDITOR := vultr-editor

BUILD ?= build
LIBS := libs
DEP ?= .dep

ENGINE_SRCS := src/sources.cpp $(wildcard vendor/**/*.cpp)
ENGINE_OBJS := $(addprefix $(BUILD)/,$(ENGINE_SRCS:%.cpp=%.o))
ENGINE_DEPS := $(addprefix $(DEP)/,$(ENGINE_SRCS:%.cpp=%.d))

EDITOR_SRCS := src/editor/sources.cpp src/platform/entry_point/linux_main.cpp
EDITOR_OBJS := $(addprefix $(BUILD)/,$(EDITOR_SRCS:%.cpp=%.o))
EDITOR_DEPS := $(addprefix $(DEP)/,$(EDITOR_SRCS:%.cpp=%.d))

ENGINE_LIBS := $(notdir $(wildcard libs/*.a))

LDFLAGS := -lvulkan -lGL -lX11 -lXxf86vm -lXrandr -lpthread -lXi -ldl -luuid -lminizip -L$(BUILD) -l$(ENGINE)

$(BUILD)/%.o: %.cpp | $(BUILD) $(DEP)
	mkdir -p $(@D)
	mkdir -p $(DEP)/$(dir $<)
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(INCLUDES) $(DEPFLAGS)

.PHONY: all clean

%.a:
	if [ ! -d $(BUILD)/$(LIBS)/$@ ]; then \
		mkdir -p $(BUILD)/$(LIBS)/$@ && ar x --output $(BUILD)/$(LIBS)/$@ $(LIBS)/$@; \
	fi

all: $(EDITOR)

clean:
	rm -f $(ENGINE_OBJS) $(EDITOR_OBJS) $(BUILD)/lib$(ENGINE).a $(BUILD)/$(EDITOR) $(ENGINE_DEPS) $(EDITOR_DEPS)

$(ENGINE): $(ENGINE_OBJS) | $(ENGINE_LIBS)
	ar -crs $(BUILD)/lib$@.a $^ $(wildcard $(BUILD)/$(LIBS)/**/*.o)

$(EDITOR): $(EDITOR_OBJS) | $(ENGINE)
	$(CXX) -o $(BUILD)/$@ $^ $(LDFLAGS)


$(BUILD) $(DEP):
	@mkdir -p $@


$(ENGINE_DEPS) $(EDITOR_DEPS):

include $(wildcard $(ENGINE_DEPS)) $(wildcard $(EDITOR_DEPS))
