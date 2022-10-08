# Vultr Game Engine
![Image](/documentation/vultr_demo_image.png)

This is my homebrewed custom game engine. It has support for basic 3D rendering with PBR and a suite of editor features that allow for basic scene editing.

This is NOT meant to create actual games, it is a hobby engine that I made for fun and learning purposes.

Features:
- Custom ECS system
- Hot-reload gameplay DLL
- Move entities around scene
- Scene serialization
- Macro-based reflection system
- STL replacement
- Vulkan 3D rendering backend
- PBR
- Shader hot-reloading
- File browser
- Multi-threaded asset loading system

# Why?

Why bother creating another game engine if you can just Unity or Unreal or \<insert favorite engine here\>? I really love game development, specifically computer graphics and I think it's a really useful exercise to build one yourself. By doing so, you can gain a better insight into how other game engines (and even other games) _really_ work. Plus it's way more fun for me to build things from scratch!

# Demo
![Demo](/documentation/vultr_demo.mp4)


# Architecture

The architecture of this engine is relatively simple. There are two projects: the actual engine itself which is in this repository and the game logic. This engine repository can be built into both an editor executable and a static lib. Then, your game logic can be built into a shared library (either a DLL or shared object file) linked to the engine static lib. The editor executable will then load this gameplay DLL and you will be able to see your game. This allows the editor to hot-reload new gameplay changes without you having to restart the editor. This is basically how all hot-reload systems work.

VultrEditor requires five function symbols to be accessible to actually load the gameplay module: `use_game_memory(void *memory)`, `void vultr_register_components()`, `void *vultr_init(void)`, `void vultr_update(void *state, f64 dt)`, and `void vultr_destroy(void *state)`. Each of these functions needs to be marked as `extern "C"` so that the names aren't mangled by C++ and the editor can load them. Each of these functions serve a purpose described below:

- `use_game_memory(void *memory)`: Because the engine uses arena memory allocation (allocates all memory is allocated from the system once at the beginning of the runtime), the gameplay needs to know where this memory is. This function will set all of the pointers to where the different allocators are. This allows all memory to be allocated on the side of the executable and to stay alive even after unloading the gameplay DLL.
- `void vultr_register_components(void)`: This engine uses an ECS architecture, and because you are able to define components in your gameplay code, you need to be able to register them with the ECS system _at runtime_. Thus this function will get called when the editor wants you to register all of your ECS components. The engine also makes a distinction between compile time, built-in components and those registered by the user at runtime so that it can easily unload the gameplay components only and reload them when a new DLL is hot-reloaded.
- `void *vultr_init(void)`: This init function is called at the start and should be where you initialize all of your state. You should also allocate all of the gameplay state you will need throughout the runtime here and return it as a `void *`. What the engine will do is that it will hold onto this pointer and pass it in whenever the update function is called.
- `void vultr_update(void *state, f64 dt)`: This update function is where you can actually update all of your gameplay (modify components, run systems, etc). The `state` parameter is simply a pointer to the state that you returned in the `void *vultr_init(void)` function. `dt` is just your delta-time.
- `void vultr_destroy(void *state)`: This is simply the cleanup function, nothing really special needs to occur here unless you want it to. The big thing is that you should `v_free` the `state` which you allocated in the `vultr_init` function.

## Rendering

When I started working on this engine, the original intent was to add multiple rendering backends so that MacOS, Linux, and Windows could be supported. Thus, I implemented a simple platform abstraction that allows you to have a simple API and at compile time specify what backend to use. This however complicated things because I wanted the rendering API to be very immediate mode and simple to use. The result was `src/platform/rendering.h` which is an ultra small (~700 LOC) header that implemented most of what I needed to render in 3D. The actual implementation for the Vulkan backend in `src/platform/rendering/vulkan/*` implements all of those APIs and abstracts much of the Vulkan boilerplate. There were many challenges that I had to over-come when implementing this abstraction. 

One noteable problem is how graphics pipelines work. I had gotten used to how OpenGL does pipelines which is that you do simple calls to configure the pipeline when you feel like it, so when I moved to Vulkan I found it incredibly cumbersome to manage pipelines that had to be pre-allocated. It made it even harder for me that pipelines had to be recreated whenever framebuffers or shaders were destroyed, since the pipeline was tied to both. I wanted an OpenGL-like abstraction to graphics pipelines that would allow me to say whether depth testing was enabled without having to deal with allocating and freeing a bunch of pipelines for all of my framebuffers. My solution was to create a `GraphicsPipelineInfo` struct that would contain all of the configuration I cared about, and to use that as a key into a hash map owned by each framebuffer. The value of this hashmap was _another_ hashmap that would use a shader pointer as a key to the actual graphics pipeline value. The logic behind this is that when the framebuffer is destroyed, all of the associated graphics pipelines are destroyed without any unnecessary look-ups. So when you bind a graphics pipeline, it will go look for an existing one in the currently bound framebuffer, and use that _or_ create one if it doesn't exist. Then when shaders are destroyed, they internally have a list of framebuffers that they need to go and notify to destroy all of their associated pipelines. This solves the shared ownership issue and allows for a very clean API to bind shaders, framebuffers, and graphics pipelines with automatic resource cleanup.

## Allocation

For this engine, I decided to opt for custom allocation, specifically using arena memory allocation. Why? It was fun/a good learning experience. My allocators don't perform _that much_ better than those of the OS (which is probably why most games don't bother with doing custom allocation), but I thought it was fun to develop. Again, arena memory allocation simply means that we allocate from the OS once and then use that memory as we wish.

The three kinds of allocators I implemented:
- Linear: I section off a small portion of the memory to be used for linear allocation, mainly for global systems that are used throughout the entire lifetime of the application. This allocator is the simplest and I literally just bump a pointer every time you allocate. This allocator isn't used automatically, and you need to explicitly reference the linear allocator when allocating.
- Slab: The slab allocator was actually a lot of fun implementing. How it works is that you designate a certain amount of memory for the slab allocator and specify the sizes of allocation you want. The memory is then sectioned off and at the front of the slab allocator lies an atomic bitfield that indicates what slabs are allocated and which are free. This allows for a completely lock-free system where you can atomic fetch free blocks and atomic set certain bits when you allocate. This allocator will get used when you allocate memory that is small enough that it can hold it, so it gets used for most allocations. This actually has a decent performance improvement over my OS allocator.
- Free-list: The last allocator I used is actually fairly slow, and was pretty hard to implement. It's a free-list allocator that uses a red-black tree to efficiently find the smallest free-block and reduce fragmentation of the heap. This allocator is used when none of the other ones work (it's too big for the slab allocator). This is used mainly for assets.

The last two allocators are automatically chosen by the system, where I implement two functions, `T *v_alloc<T>(size_t count = 1)` and `void v_free<T>(T *memory)`. Instead of specifying a size, these methods will just use the size of the template type to figure out how much to allocate. These essentially just replace `new` and `delete`, because they also call the corresponding constructors and destructors.

## C-like C++

During the development of this engine, I experimented with many different design patterns. In C++, there are many different philosophies on what is "good practice," ranging from banning raw pointers and only use smart pointers, to using almost no C features and using the C++ versions entirely (no structs, only classes), to writing C++ like Java with abstract classes interfaces with multiple inheritance throughout the entire codebase. I opted for something that goes in-between all of them, seeking to not be dogmatic but instead to see what patterns looked the most optimal to me and my use case. This is a single developer project, and there are simply more efficient and readable patterns you can use if you know exactly the skill level of all of the developers that will ever be on your team and there's no need to make black-box abstractions (since you wrote all the code). So I chose to write in essentially a C-like C++. This is a pretty common design pattern, and it essentially entails using free functions, structs, no private and public, no smart pointers, no STL, etc. to boil down C++ and make it far simpler to read and write. The benefit of this is twofold: performance improves because you aren't relying on many of the abstractions that C++ provides which are not zero cost (unlike something like Rust where they are indeed zero cost abstractions) and in addition you are able to write code in a far more readable and simple way that more resembles functional and procedural programming. I do use a decent number of C++ features, notably namespaces, templates, const references, compile-time constexpr and consteval functions and data, and operator overloading. Aside from that, it mostly reads like C code.

The design philosophy behind only using free functions stems from the idea that data should be separated from behavior. So bundling the two into objects with the syntactical sugar of "object does verb" doesn't really make much sense in this view. If we change this to, "Do some action on input data x, y, z with output w," the behavior is a lot simpler and extensible. Instead of making the problem fit OOP, we make our code solve the problem directly. There is however a notable exception to this, my template library. Data structures actually are pretty much bundled with their behavior, and they are not going to be used without their specified behavior. A hashmap has a specific behavior when inserting a key-value pair, and that's not ever going to change. And you can't use a hashmap data structure without that behavior. As such, my custom data structures have methods and private member variables (I don't use class because it's a redundant keyword and I don't see the use).

## No STL??

Compile-times really matters to me, and what I found was that by simply using the STL types, my compile times sky-rocketed (for example, just using `std::unordered_map` slowed down my compile times significantly and could be seen in the clang analyzer). So I implemented a very light-weight template library that uses my own custom allocators and adds support for many performance optimizations such as inline vector buffers on the stack and other things. Many of the implementations are also based off of [SerenityOS's AK custom standard library](https://github.com/SerenityOS/serenity/tree/master/AK).

## Safety

C++ often claims that by using features such as smart pointers, RAII, and the like you will be able to write safe code (which I agree with). However, at the same time, if you read most C++ code you'll see things like, `std::unique_ptr<SomeType> unintialized_member = nullptr;`. And then in a later member, that member will get initialized. This behavior is one of the biggest disasters of C++. If you look into a lot of the modern languages, like C#, Swift, and notably Rust, you will see that when you have something "nullable," you must explicitly "unwrap" it. C++ has the `optional` type, but almost no one actually uses it for some god-forsaken reason, and it causes all sorts of issues. I opted to simply ban the use of `nullptr` for my own input and output where possible. What we mean when we say, "nullable" really is "optional," and as such I implement an `Option<T>` type that allows you to explicitly unwrap optional types.
