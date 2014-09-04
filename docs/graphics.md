# IntenseLogic Graphics Architecture

## Context

The graphics library starts out with `ilG_context`. The context contains state
related to the render thread and common state among renderers. It is passed to
all renderers when they are constructed.

The context structure has a long history, but its current purpose is mainly to
contain the state associated with the render thread.

The context acts somewhat like an Actor as in the Actor Model, in that it is
self-contained and mainly communicates through message passing.

Fields of the context:

- A sliding window of frames, to keep track of FPS
- Event handlers for frame rendering (`ilG_context::tick`), window resizing
  (`ilG_context::resize`), close window requests (the x button,
  `ilG_context::close`), and when the context is being destroyed
  (`ilG_context::destroy`)
- Two message queues, one for messages to the render thread, and one back to the
  client
- A Render Manager, used to contain state for [Renderers][]
- A Frame buffer for drawing all of the renderers into, allowing post-processing,
  anti-aliasing, and the G-buffer required for deferred shading
- Several context creation hints, such as which GL version to use, whether to
  enable HDR/MSAA/vsync, the window title

## Renderers

Renderers are a group of types including `ilG_renderer` which compose a system
with which to render objects on the screen. They are stored in a
`ilG_rendermanager`. There are three types of renderers:

- Static renderers, which do not rely on world state, such as GUIs
- Viewspace renderers, which rely on the camera, but no in-world objects, such
  as skyboxes
- Object renderers, which render batches of objects.

Renderers have the following optional properties:

- Child renderers which are rendered immediately after the parent
- Attached lights which may be iterated over
- Attached message sink, which can process messages from the client thread
- Attached names, for debugging and introspection
- Attached `il_table` storage, should it be required

There are two ways to create a renderer. One is directly on the render thread,
and the other is from the client. First, you construct an `ilG_builder` which
contains a `ilG_builder::build` method and a pointer to any attached data needed
for rendering.

On the client thread, a renderer can be created by passing the builder to
`ilG_build()`, which will return a handle which you can pass to functions
defined in `renderer.h`, which will be sent as messages through the context's
message queue.

On the render thread, a renderer can be created by passing the builder to
`ilG_context_addRenderer()`, which will simply return an index into the
`ilG_rendermanager::renderers` and `ilG_rendermanager::rendids` arrays inside
of the `ilG_rendermanager`. Obtaining the `ilG_rendid` from the `rendids` array
can be done to call the management functions defined in `context.h`, which are
the render-thread analogues to the ones in `renderer.h`.

Renderers exist in this way because it allows parts which are optional but
common to many renderers to exist without boilerplate, to reduce the number of
NULL pointers, and to be cache-efficient while performing various operations.
For more information on the architecture of this, see
[the book on Data-Oriented Design](http://www.dataorienteddesign.com/dodmain/).

The handle type presented for the client thread is present to allow manipulation
of the scene-graph on the render thread without having to use locking mechanisms
and open up opportunity for race conditions.

## Materials

Materials are a wrapper about OpenGL Shaders intended for use by Renderers.

Much of the functionality they used to provide is no longer necessary, so the
abstraction is fairly thin. In the future, this will likely be superseded
entirely by a shader abstraction in
[TiffGL](https://github.com/tiffany352/tiffgl/).

## Textures

These provide helper functions for creating OpenGL Textures and binding them.
In the future, this functionality will likely be revised and moved to TiffGL.

## Provided rendering primitives

These are not fully fledged renderers, but may be used by them, much like
materials. These will be moved to TiffGL in the future.

- Shapes: Supports box and icosahedron primitives.
- Meshes: Helper to convert `ilA_mesh` to something that can be drawn.

## Coordinate Spaces

One feature that is needed in IL is the ability to have arbitrarily large
worlds, but some games don't need such functionality. In order to accommodate
this without loss of performance, IL has an interface called `ilG_coordsys`,
which allows you to shim in new coordinate systems.

Coordinate systems work by maintaining all of the state of objects in the world,
the camera, and the projection matrix themselves. The engine simply asks for a
list of matrices in various formats (combinations of flags from `transform.h`)
for a given list of object IDs. Object IDs are assigned to renderers by the
client, and the client should obtain the IDs from the coordinate system's client
side interface (this part is not abstracted by IL).

A default implementation of this interface is provided by `floatspace.h`, which
uses single-precision floating point numbers to position objects in the world.
There is no requirement to use floating point in your own coordinate systems,
fixed point positions are entirely possible.

The only requirement is that the position be translatable to a 4x4 floating
point matrix. This requirement is not a large one - it does not introduce
artifacts at great distances. You do your world-space translation (from world
space to camera space) in the native format, and then create the matrix. There
will technically be artifacts if an object is a great distance from the camera,
but because the distance of the object will scale the size of the object on the
screen down as it gets far away, it will not be visible.

## Provided Renderers

- Height-maps: You simply provide a heightmap texture, a normal map, and a
  texture with which to color the ground. This is used by the bouncing lights
  demo to create the ground.
- Lighting pass: This implements deferred shading. Two options are available
  point light shading, and sunlight shading. Sunlight shading uses full-screen
  quads, which is useful for lights which are larger than the view frustum.
- Geometry pass: This sets some GL options such as depth testing, and all
  world-space object rendering should take place as a child of this pass.
- Output pass: This outputs from the screen frame-buffer to the actual window.
  This pass must be present if the `ilG_context::use_default_fb` hint is not set.
- Screen-shot pass: This will output a copy of the frame-buffer into a provided
  image, which can be saved as PNG file using functions provided in the asset
  library.
- Transparency pass: Used for rendering non-solid objects. Should take place
  after the geometry pass.
