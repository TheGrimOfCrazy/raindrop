#include <map>

#include "Global.h"
#include "Song.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/foreach.hpp>

int SectionIndex(const TimingData &Timing, double Beat)
{
	int Index = -1;
	for (TimingData::const_iterator i = Timing.begin(); i != Timing.end(); i++)
	{
		if ( Beat >= i->Time )
			Index++;
		else
			break;
	}
	return Index;
}

double SectionValue(const TimingData &Timing, double Beat)
{
	if (Beat < Timing[0].Time)
		return Timing[0].Value;
	else
	{
		int Index = SectionIndex(Timing, Beat);
#ifndef NDEBUG
		if (Index < 0)
			Utility::DebugBreak();
#endif
		return Timing[Index].Value;
	}
}

double TimeAtBeat(const TimingData &Timing, float Offset, double Beat)
{
	uint32 CurrentIndex = SectionIndex(Timing, Beat) + 1;
	double Time = Offset;

	if (Beat == 0) return Time;

	for (uint32 i = 0; i < CurrentIndex; i++)
	{	
		if (i+1 < CurrentIndex) // Get how long the current timing goes.
		{
			float BeatDurationOfSectionI = Timing[i+1].Time - Timing[i].Time; // Section lasts this much.
			float SectionDuration = BeatDurationOfSectionI * spb ( Timing[i].Value );

			if (Beat < Timing[i+1].Time && Beat > Timing[i].Time)
			{
				// If this is our interval, stop summing time of previous intervals before this one
				SectionDuration = (Beat - Timing[i].Time) * spb ( Timing[i].Value );
			}
			Time += SectionDuration;
		}else
			Time += (Beat - Timing[i].Time) * spb ( Timing[i].Value );
	}
	return Time;
}

void GetTimingChangesInInterval(const TimingData &Timing, 
	double PointA, double PointB, 
	TimingData &Out)
{
	Out.clear();

	for (TimingData::const_iterator i = Timing.begin(); i != Timing.end(); i++)
	{
		if (i->Time >= PointA && i->Time < PointB)
		{
			Out.push_back(*i);
		}
	}
}

void LoadTimingList(TimingData &Timing, GString line, bool AllowZeros)
{
	GString ListGString = line.substr(line.find_first_of(":") + 1);
	std::vector< GString > SplitResult;
	TimingSegment Segment;

	// Remove whitespace.
	boost::replace_all(ListGString, "\n", "");
	boost::split(SplitResult, ListGString, boost::is_any_of(",")); // Separate List of BPMs.
	BOOST_FOREACH(GString ValueGString, SplitResult)
	{
		std::vector< GString > SplitResultPair;
		boost::split(SplitResultPair, ValueGString, boost::is_any_of("=")); // Separate Time=Value pairs.

		if (SplitResultPair.size() == 1) // Assume only one BPM on the whole list.
		{
			Segment.Value = atof(SplitResultPair[0].c_str());
			Segment.Time = 0;
		}else // Multiple BPMs.
		{
			Segment.Time = atof(SplitResultPair[0].c_str());
			Segment.Value = atof(SplitResultPair[1].c_str());
		}

		if (AllowZeros || Segment.Value)
			Timing.push_back(Segment);
	}
}

double StopTimeAtBeat(const TimingData &StopsTiming, double Beat)
{
	double Time = 0;

	if (Beat == 0 || !StopsTiming.size()) return Time;

	for (uint32 i = 0; i < StopsTiming.size(); i++)
	{	
		if (StopsTiming.at(i).Time < Beat)
			Time += StopsTiming.at(i).Value;
	}

	return Time;
}

double IntegrateToTime(const TimingData &Timing, double Time, float Drift)
{
	int Section = SectionIndex(Timing, Time);
	double Out = 0;

	if (Section == -1) // Time is behind all.
	{
		Out = - (Timing[0].Time - Time) * Timing[0].Value;
	}else // Time comes after first entry.
	{
		for (uint32 i = 0; i < Section; i++)
			Out += (Timing[i+1].Time - Timing[i].Time) * Timing[i].Value;

		Out += (Time - Timing[Section].Time + Drift) * Timing[Section].Value;
	}

	return Out;
}

double QuantizeFractionBeat(float Frac)
{
	return double (min (48.0, floor(Frac * 49.0))) / 48.0;
}

double QuantizeFractionMeasure (float Frac)
{
	return  (min (192.0, floor(Frac * 193.0))) / 192.0;
}

double QuantizeBeat(float Beat)
{
	double dec = QuantizeFractionBeat(Beat - floor(Beat));
	return dec + floor(Beat);
}

#define FRACKIND(x,y) if(Row%x==0)fracKind=y

int GetFractionKindMeasure(double frac)
{
	int fracKind = 1;
	int Row = QuantizeFractionMeasure (frac);

	if (!Row) return 4;

	FRACKIND(2,96);
	FRACKIND(3,64);
	FRACKIND(4,48);
	FRACKIND(6,32);
	FRACKIND(8,24);
	FRACKIND(12,16);
	FRACKIND(16,12);
	FRACKIND(24,8);
	FRACKIND(32,6);
	FRACKIND(48,4);
	FRACKIND(64,3);
	FRACKIND(96,2);

	return fracKind;
}

int GetFractionKindBeat(double frac)
{
	int fracKind;
	int Row = QuantizeFractionBeat (frac);

	FRACKIND(1, 48);

	// placed on 1/24th of a beat
	FRACKIND(2, 24);

	// placed on 1/16th of a beat
	FRACKIND(3,16);

	// placed on 1/8th of a beat
	FRACKIND(6,8);

	// placed on 1/6th of a beat
	FRACKIND(8,6);

	// placed on 1/4th of a beat
	FRACKIND(12,4);

	// placed on 1/3rd of a beat
	FRACKIND(16,3);

	// placed on 1/2nd of a beat
	FRACKIND(24,2);

	// placed on 1/1 of a beat
	FRACKIND(48,1);

	return fracKind;
}