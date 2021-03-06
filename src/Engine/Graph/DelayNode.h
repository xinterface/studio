/****************************************************************************
**
** Copyright 2019 neuromore co
** Contact: https://neuromore.com/contact
**
** Commercial License Usage
** Licensees holding valid commercial neuromore licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and neuromore. For licensing terms
** and conditions see https://neuromore.com/licensing. For further
** information use the contact form at https://neuromore.com/contact.
**
** neuromore Public License Usage
** Alternatively, this file may be used under the terms of the neuromore
** Public License version 1 as published by neuromore co with exceptions as 
** appearing in the file neuromore-class-exception.md included in the 
** packaging of this file. Please review the following information to 
** ensure the neuromore Public License requirements will be met: 
** https://neuromore.com/npl
**
****************************************************************************/

#ifndef __NEUROMORE_DELAYNODE_H
#define __NEUROMORE_DELAYNODE_H

// include the required headers
#include "../Config.h"
#include "../Core/StandardHeaders.h"
#include "ProcessorNode.h"
#include "../DSP/ChannelProcessor.h"


class ENGINE_API DelayNode : public ProcessorNode
{
	public:
		enum { TYPE_ID = 0x0027 };
		static const char* Uuid () { return "f903a81e-e39a-11e4-8a00-1681e6b88ec1"; }
		
		//
		enum
		{
			INPUTPORT_VALUE		= 0,
			OUTPUTPORT_RESULT	= 0
		};

		enum
		{
			ATTRIB_DELAY_SETBYTIME = 0,
			ATTRIB_DELAY_DURATION,
			ATTRIB_DELAY_NUMSAMPLES,
		};

		// constructor & destructor
		DelayNode(Graph* graph);
		~DelayNode();

		// initialize & update
		void Init() override;
		void ReInit(const Core::Time& elapsed, const Core::Time& delta) override;
		void Update(const Core::Time& elapsed, const Core::Time& delta) override;
		void Start(const Core::Time& elapsed) override;

		void OnAttributesChanged() override;
		
		Core::Color GetColor() const override								{ return Core::RGBA(143,255,33); }
		uint32 GetType() const override											{ return TYPE_ID; }
		const char* GetTypeUuid() const override final							{ return Uuid(); }
		const char* GetReadableType() const override							{ return "Delay"; }
		const char* GetRuleName() const override final							{ return "NODE_Delay"; }
		uint32 GetPaletteCategory() const override								{ return CATEGORY_UTILS; }
		GraphObject* Clone(Graph* graph) override								{ DelayNode* clone = new DelayNode(graph); return clone; }

		const ChannelProcessor::Settings& GetSettings() override				{ return mSettings; }

	private:
		void SetDelayBySamples(uint32 delayInSamples);
		void SetDelayByTime(double delayInSeconds);

		// settings for the node processor
		class ProcessorSettings : public ChannelProcessor::Settings
		{
			public:
				enum { TYPE_ID = 0x0027 };

				ProcessorSettings()			 {}
				virtual ~ProcessorSettings() {}
			
				uint32 GetType() const override		{ return TYPE_ID; }

				uint32 mDelayInSamples;
				double mDelayInSeconds;
				bool   mSetByTime;
		};
		
		ProcessorSettings	mSettings;

		// the node's channel processor
		class Processor : public ChannelProcessor
		{
			enum { TYPE_ID = 0x0027 };

			public:
				Processor() : ChannelProcessor()										{ Init();  }
				~Processor() { }

				uint32 GetType() const override											{ return TYPE_ID; }
				ChannelProcessor* Clone() override										{ Processor* clone = new Processor(); clone->Setup(mSettings); return clone; }

				// settings
				void Setup(const ChannelProcessor::Settings& settings) override			{ mSettings = static_cast<const ProcessorSettings&>(settings); }
				const Settings& GetSettings() const	override							{ return mSettings; }

				// properties
				uint32 GetDelay(uint32 inputPortIndex, uint32 outputPortIndex) const override	{ return mSettings.mDelayInSamples; }
				uint32 GetNumStartupSamples (uint32 inputPortIndex) const override				{ return mSettings.mDelayInSamples; }
				uint32 GetNumEpochSamples(uint32 inputPortIndex) const override					{ return mSettings.mDelayInSamples+1; /*always one more for reading*/ }

				void Init() override
				{
					AddInput<double>();
					AddOutput<double>();
				}

				void ReInit() override;
				void Update() override;

			private:
				ProcessorSettings		mSettings;
		};
};


#endif
