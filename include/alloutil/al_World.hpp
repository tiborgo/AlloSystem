#ifndef AL_WORLD_HPP_INC
#define AL_WORLD_HPP_INC

#include <math.h>
#include <string>
#include <vector>

#include "allocore/al_Allocore.hpp"
#include "alloutil/al_ControlNav.hpp"

namespace al{


/// Viewpoint within a scene
class Viewpoint{
public:

	Viewpoint()
	:	mViewport(0,0,0,0),
		mAnchorX(0), mAnchorY(0), mStretchX(1), mStretchY(1),
		mCamera(0)
	{}

	float anchorX() const { return mAnchorX; }
	float anchorY() const { return mAnchorY; }
	float stretchX() const { return mStretchX; }
	float stretchY() const { return mStretchY; }

	bool hasCamera() const { return 0 != mCamera; }

	const Camera& camera() const { return *mCamera; }

	const Pose& transform() const { return mTransform; }
	Pose& transform(){ return mTransform; }
	
	const Viewport& viewport() const { return mViewport; }
	Viewport& viewport(){ return mViewport; }

	Viewpoint& anchor(float ax, float ay){
		mAnchorX=ax; mAnchorY=ay; return *this;
	}

	Viewpoint& stretch(float sx, float sy){
		mStretchX=sx; mStretchY=sy; return *this;
	}

	Viewpoint& camera(Camera& v){
		mCamera=&v; return *this;
	}

protected:
	Viewport mViewport;
	Pose mTransform;
	float mAnchorX, mAnchorY;
	float mStretchX, mStretchY;
	Camera * mCamera;
};



class Actor : public SoundSource {//, public gfx::Drawable {
public:

	virtual ~Actor(){}

	/// Sound generation callback

	/// Sound can either be written directly to the audio output channels
	/// or to the sound source's internal buffer which is rendered later by the
	/// spatial audio decoder.
	virtual void onSound(AudioIOData& io){}

	/// Animation (model update) callback
	virtual void onAnimate(double dt){}

	/// Drawing callback
	
	/// This will be called from the main graphics renderer. Since it may be 
	/// called multiple times, no state updates should be made in it.
	virtual void onDraw(Graphics& g, const Viewpoint& v){}

protected:
};


class Spectator : public Listener {
public:
	typedef std::vector<Viewpoint *> Viewpoints;

protected:
};


class AlloView : public Window {
public:
	using Window::add;
	typedef std::vector<Viewpoint *> Viewpoints;

	AlloView(){
		add(new StandardWindowKeyControls);
		add(new ResizeHandler(*this));
	}
	
	AlloView(
		int l, int t, int w, int h,
		const std::string title,
		double fps=40,
		DisplayMode::t mode = DisplayMode::DefaultBuf
	){
		add(new StandardWindowKeyControls);
		add(new ResizeHandler(*this));
		create(Dim(l,t,w,h), title, fps, mode);
	}

	const Viewpoints& viewpoints() const { return mViewpoints; }
	
	void add(Viewpoint& v){ mViewpoints.push_back(&v); }

protected:
	Viewpoints mViewpoints;
	
	struct ResizeHandler : public WindowEventHandler{
		ResizeHandler(AlloView& v_): v(v_){}
		bool onResize(int dw, int dh){
			Viewpoints::iterator iv = v.mViewpoints.begin();
			
			while(iv != v.mViewpoints.end()){
				Viewpoint& vp = **iv;

				vp.viewport().l += dw * vp.anchorX();
				vp.viewport().b += dh * vp.anchorY();
				vp.viewport().w += dw * vp.stretchX();
				vp.viewport().h += dh * vp.stretchY();

				++iv;
			}
			return true;
		}
		AlloView& v;
	};
};



class World {
public:
	
	World(const std::string& name="")
	:	mGraphics(new GraphicsBackendOpenGL),
		mAudioIO(128, 44100, sAudioCB, this, 2, 1),
		mAudioScene(3,2, 128),
		mName(name)
	{
		mListeners.push_back(&mAudioScene.createListener(2));
	}

	const std::string& name() const { return mName; }

	const Camera& camera() const { return mCamera; }
	Camera& camera(){ return mCamera; }

	const Nav& nav() const { return mNav; }

	World& name(const std::string& v){ mName=v; return *this; }

	void add(Actor& v){
		mActors.push_back(&v);
	}
	
	void add(AlloView& v, bool animates=false);
	
	void start(){
		mAudioIO.start();
		MainLoop::start();
	}
	
	Stereographic& stereo(){ return mStereo; }

protected:

	typedef std::vector<Actor *> Actors;
	typedef std::vector<Listener *> Listeners;
	typedef std::vector<AlloView *> Viewports;
	
	Actors mActors;
	Listeners mListeners;
	Viewports mViewports;
	Nav mNav;
	Camera mCamera;

	Stereographic mStereo;
	Graphics mGraphics;
	AudioIO mAudioIO;
	AudioScene mAudioScene;

	std::string mName;

	static void sAudioCB(AudioIOData& io){
		World& w = io.user<World>();
		int numFrames = io.framesPerBuffer();
		
		//w.mNavMaster.velScale(4);
		//w.mNavMaster.step(io.secondsPerBuffer());
		w.mNav.smooth(0.95);
		w.mNav.step(1./4);

		Actors::iterator it = w.mActors.begin();

		while(it != w.mActors.end()){
			io.frame(0);
			(*it)->onSound(w.mAudioIO);
			++it;
		}

		w.mAudioScene.encode(numFrames, io.framesPerSecond());
		w.mAudioScene.render(&io.out(0,0), numFrames);
		//printf("%f\n", io.out(0,0));
	}

	// This is called by each window to render all actors in each of its view regions
	struct DrawActors : public WindowEventHandler{
		DrawActors(AlloView& v_, World& w_): v(v_), w(w_){}

		virtual bool onFrame(){

			Graphics& g = w.mGraphics;
			//int w = v.dimensions().w;
			//int h = v.dimensions().h;
			
			g.depthTesting(true);
			
			//printf("%f %f %f\n", navMaster.x(), navMaster.y(), navMaster.z());
			//printf("%p: %f %f %f\n", &v.camera(), cam.x(), cam.y(), cam.z());

			struct DrawAllActors : public Drawable {
				DrawAllActors(Actors& as_, World& w_, Viewpoint& v_)
					:	as(as_), w(w_), v(v_){}
				virtual void onDraw(Graphics& g){
					Actors::iterator ia = as.begin();
					while(ia != as.end()){
						Actor& a = *(*ia);
						g.matrixMode(g.MODELVIEW);
						g.pushMatrix();
						g.multMatrix(a.matrix());
						a.onDraw(g,v);
						g.matrixMode(g.MODELVIEW);
						g.popMatrix();
						++ia;
					}
				}
				Actors& as;
				World& w;
				Viewpoint& v;
			};

			AlloView::Viewpoints::const_iterator iv = v.viewpoints().begin();
			
			while(iv != v.viewpoints().end()){
				Viewpoint& vp = *(*iv);
				
				// if no camera, set to default scene camera
				if(!vp.hasCamera()) vp.camera(w.camera());

				const Camera& cam = vp.camera();
				DrawAllActors drawFunc(w.mActors, w, vp);
				w.mStereo.draw(g, cam, w.nav() * vp.transform(), vp.viewport(), drawFunc);
				++iv;
			}

			return true;
		}

		AlloView& v;
		World& w;
	};
	
	struct AnimateActors : public WindowEventHandler{
	
		AnimateActors(World& w_): w(w_){}
	
		virtual bool onFrame(){
			Actors::iterator ia = w.mActors.begin();
			while(ia != w.mActors.end()){
				Actor& a = *(*ia);
				a.step();
				a.onAnimate(window().spf());
				++ia;
			}
			return true;			
		}
		World& w;
	};
	
	struct SceneInputControl : public InputEventHandler{
		SceneInputControl(World& w_): w(w_){}
		
		virtual bool onKeyDown(const Keyboard& k){
			switch(k.key()){
				case Key::Tab: w.stereo().stereo(!w.stereo().stereo()); return false;
				default:;
			}
			return true;
		}

		World& w;
	};

};



//class Clocked{
//public:
//	virtual void onUpdate(double dt){}
//protected:
//};
//
//
//class Clock{
//public:
//
//	void add(Clocked& v){ mListeners.push_back(&v); }
//
//	void update(double dt){
//		Listeners::iterator it = mListeners.begin();
//		while(it != mListeners.end()){
//			(*it)->onUpdate(dt);
//		}
//	}
//
//protected:
//	typedef std::vector<Clocked *> Listeners;
//	Listeners mListeners;
//};

} // al::
#endif