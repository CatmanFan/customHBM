#ifndef __HBM__HBMElement__
#define __HBM__HBMElement__

class HBMElement {
	private:
		f32 Timer1;
		f32 Timer2;
		f64 TimeSnapshot;
		bool TimerRunning;
		bool HitboxTouched(int chan);
		u8 PendingAnimation;

	protected:
		class HBMImage Shadow;
		float ShadowOpacity;
		int ShadowX;
		int ShadowY;
		float Scale = 1.0F;

		//! Hitbox is the area designated to be controlled by the IR pointer.
		struct {
			int X;
			int Y;
			int Width;
			int Height;
		} Hitbox;

		//! Secondary hitbox, for elements such as the Wii Remote diagram.
		struct {
			int X;
			int Y;
			int Width;
			int Height;
		} Hitbox2;

		/**
		 * Checks hitbox status.
		 *
		 * @param   conditions Additional conditions to be met, otherwise the hitbox will be treated as inactive. Can be bypassed with "true".
		 * @return  In hexadecimal form, both the IR controller number activating the hitbox and whether the hitbox is hovered or pressed
		 *          (e.g. 0x10 indicates that Player 1 has pressed the element). Otherwise, -1 if the hitbox is currently inactive.
		 */
		u8 HitboxStatus(bool conditions);

		//! The status of the button based on the IR pointer's interaction with the hitbox
		u8 Status;

		//! Timer options and functions.
		bool TimerActive()						{ return this->TimerRunning; }
		void TimerUpdate()						{
													if (this->TimerRunning) {
														if (this->Timer1 > 0) {
															this->Timer1 = (f64)HBM_GETTIME / 1000.0F;

															// Hotfix for negative timer values
															while (this->Timer1 - this->TimeSnapshot < -1 || this->TimeSnapshot == 0) {
																extern void HBM_ConsolePrintf(const char* msg, ...);
																HBM_ConsolePrintf("obj timer is negative!! resetting");
																HBM_ConsolePrintf("(timer 1: %.5f)", this->Timer1);
																HBM_ConsolePrintf("(timer snap: %.5f)", this->TimeSnapshot);
																this->TimerStart(true);
															}
														}

														if (this->Timer2 > 0)
															this->Timer2 = (f64)HBM_GETTIME / 1000.0F;
													}
												}
		void TimerStart(bool force = false)		{
													if (!this->TimerRunning || force) {
														this->TimeSnapshot = (f64)HBM_GETTIME / 1000.0F;
														this->Timer1 = (f64)HBM_GETTIME / 1000.0F;
														this->TimerRunning = true;
													}
												}
		void TimerStop()						{
													if (this->TimerRunning) {
														this->Timer2 = this->Timer1 = 0;
														this->TimeSnapshot = 0;
														this->TimerRunning = false;
													}
												}
		f32 GetTimePassed()						{ return MAX(0, this->Timer1 - this->TimeSnapshot); }
		void Timer2Start()						{
													if (this->Timer2 == 0 && this->TimerRunning)
														this->Timer2 = (f64)HBM_GETTIME / 1000.0F;
												}
		void Timer2Stop()						{ if (this->Timer2 > 0) this->Timer2 = -1; }
		bool TimerPassed(f32 x)					{ return this->TimerRunning ? this->Timer1 - this->TimeSnapshot >= x : true; }
		bool Timer2Passed(f32 x)				{ return this->TimerRunning && this->Timer2 >= 0 ? this->Timer2 - this->TimeSnapshot >= x : true; }
		f32 TimerProgress(f32 x, f32 y = 0)		{
													if (y > x && y > 0)
														return this->TimerRunning ? MIN(1, MAX(0, this->Timer1 - this->TimeSnapshot - x) / (y - x)) : 0;
													else
														return this->TimerRunning ? MIN(1, MAX(0, this->Timer1 - this->TimeSnapshot) / x) : 0;
												}
		f32 Timer2Progress(f32 x, f32 y = 0)	{
													if (y > x)
														return this->TimerRunning ? MIN(1, MAX(0, this->Timer2 - this->TimeSnapshot - x) / (y - x)) : 0;
													else
														return this->TimerRunning ? MIN(1, MAX(0, this->Timer2 - this->TimeSnapshot) / x) : 0;
												}
		f32 TimerProgressEase(f32 x, f32 y = 0)	{ return HBM_EASEINOUT(TimerProgress(x, y)); }
		f32 Timer2ProgressEase(f32 x, f32 y = 0) { return HBM_EASEINOUT(Timer2Progress(x, y)); }

		int Animation;
		int AnimationPending;
		/**
		 * Plays an animation as specified in the internal element class.
		 *
		 * @param   value The index of the animation, should be the same as the IR status. If set to less than 0, it will stop the current animation.
		 * @param   force Whether to force the target animation to play instead of waiting for the current one to finish.
		 */
		void SetAnimation(int value = -1, bool force = false) {
			if (value >= 0) {
				if (force) {
					this->TimerStop();
					this->Animation = value + 1;
					this->AnimationPending = 0;
					this->TimerStart();
				} else {
					if (this->Animation != value + 1)
						this->AnimationPending = value + 1;
				}
			} else {
				this->Animation = 0;
				this->TimerStop();
			}
		}

		/**
		 * Updates the element timer and automatically schedules the pending animation to play.
		 */
		void UpdateAnimation() {
			if (!this->TimerRunning && this->AnimationPending > 0) {
				// Play scheduled animation
				this->Animation = this->AnimationPending;
				this->AnimationPending = 0;
				this->TimerStart();
			} else {
				this->TimerUpdate();
			}
		}

		bool CheckAnimation(int index) {
			return this->Animation == index + 1;
		}

		bool CheckAnimationPlaying(int index) {
			return this->Animation == index + 1 && this->TimerRunning;
		}

		bool CheckAnimationStopped(int index) {
			return this->Animation == index + 1 && !this->TimerRunning;
		}

	public:
		HBMElement();
		~HBMElement();

		class HBMImage Image;
		bool Visible; // If the element should be drawn
		bool Blocked; // If the element cannot be accessed by the pointer
		int X;
		int Y;

		virtual void Draw();
		virtual void Update();
		void SetOpacity(float value);
		void SetHitbox(int w, int h);
		void SetHitbox(int x, int y, int w, int h);
		void SetHitbox2(int w, int h);
		void SetHitbox2(int x, int y, int w, int h);
		void SetPosition(int x, int y);
		int GetX();
		int GetY();
};

#endif