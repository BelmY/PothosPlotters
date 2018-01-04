// Copyright (c) 2014-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "SpectrogramDisplay.hpp"
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <iostream>

/***********************************************************************
 * |PothosDoc Spectrogram
 *
 * The spectrogram plot displays a live plot of the spectrum vs time.
 *
 * |category /Plotters
 * |keywords frequency plot fft dft spectrum spectral
 * |alias /widgets/spectrogram
 *
 * |param title The title of the plot
 * |default "Spectrogram"
 * |widget StringEntry()
 * |preview valid
 *
 * |param displayRate[Display Rate] How often the plotter updates.
 * |default 10.0
 * |units updates/sec
 * |preview disable
 *
 * |param sampleRate[Sample Rate] The rate of the input elements.
 * |default 1e6
 * |units samples/sec
 *
 * |param centerFreq[Center Freq] The center frequency of the plot.
 * This value controls the labeling of the horizontal access.
 * |default 0.0
 * |units Hz
 * |preview valid
 *
 * |param numBins[Num FFT Bins] The number of bins per fourier transform.
 * |default 1024
 * |option 512
 * |option 1024
 * |option 2048
 * |option 4096
 * |widget ComboBox(editable=true)
 * |preview disable
 * |tab FFT
 *
 * |param window[Window Type] The window function controls passband ripple.
 * |default "hann"
 * |option [Rectangular] "rectangular"
 * |option [Hann] "hann"
 * |option [Hamming] "hamming"
 * |option [Blackman] "blackman"
 * |option [Bartlett] "bartlett"
 * |option [Flat-top] "flattop"
 * |option [Kaiser] "kaiser"
 * |option [Chebyshev] "chebyshev"
 * |preview disable
 * |tab FFT
 *
 * |param windowArgs[Window Args] Optional window arguments (depends on window type).
 * <ul>
 * <li>When using the <i>Kaiser</i> window, specify [beta] to use the parameterized Kaiser window.</li>
 * <li>When using the <i>Chebyshev</i> window, specify [atten] to use the Dolph-Chebyshev window with attenuation in dB.</li>
 * </ul>
 * |default []
 * |preview disable
 * |tab FFT
 *
 * |param fullScale[Full Scale] The amplitude that corresponds to full-scale.
 * A full-scale amplitude signal will appear as 0.0 dBfs on the plotter.
 * The default value of 1.0 works best for scaled floating point samples.
 * A signed 16-bit integer value might use 32768 as full-scale instead.
 * |default 1.0
 * |preview disable
 * |tab FFT
 *
 * |param fftMode[FFT Mode] Power spectrum display mode.
 * <ul>
 * <li>Real mode ("REAL") displays only the positive frequencies between [0, +fs/2].</li>
 * <li>Complex mode ("COMPLEX) displays positive and negative frequencies between [-fs/2, +fs/2].</li>
 * <li>Automatic mode ("AUTO") selects the FFT mode based on the data type of the current signal.</li>
 * </ul>
 * |default "AUTO"
 * |option [Automatic] "AUTO"
 * |option [Complex] "COMPLEX"
 * |option [Real] "REAL"
 * |preview disable
 * |tab FFT
 *
 * |param timeSpan[Time Span] How many seconds of data to display in the plot.
 * |default 10.0
 * |units seconds
 * |preview disable
 * |tab Axis
 *
 * |param refLevel[Reference Level] The maximum displayable power level.
 * |default 0.0
 * |units dBxx
 * |widget DoubleSpinBox(minimum=-150, maximum=150, step=10, decimals=1)
 * |preview disable
 * |tab Axis
 *
 * |param dynRange[Dynamic Range] The ratio of largest to smallest displayable power level.
 * The vertical axis will display values from the ref level to ref level - dynamic range.
 * |default 100.0
 * |units dB
 * |widget DoubleSpinBox(minimum=10, maximum=200, step=10, decimals=1)
 * |preview disable
 * |tab Axis
 *
 * |param enableXAxis[Enable X-Axis] Show or hide the horizontal axis markers.
 * |option [Show] true
 * |option [Hide] false
 * |default true
 * |preview disable
 * |tab Axis
 *
 * |param enableYAxis[Enable Y-Axis] Show or hide the vertical axis markers.
 * |option [Show] true
 * |option [Hide] false
 * |default true
 * |preview disable
 * |tab Axis
 *
 * |param colorMap[Color Map] The name of a color map for the raster plot.
 * |widget ColorMapEntry()
 * |default "rainbow"
 * |preview disable
 * |tab Axis
 *
 * |param freqLabelId[Freq Label ID] Labels with this ID can be used to set the center frequency.
 * To ignore frequency labels, set this parameter to an empty string.
 * |default "rxFreq"
 * |widget StringEntry()
 * |preview disable
 * |tab Labels
 *
 * |param rateLabelId[Rate Label ID] Labels with this ID can be used to set the sample rate.
 * To ignore sample rate labels, set this parameter to an empty string.
 * |default "rxRate"
 * |widget StringEntry()
 * |preview disable
 * |tab Labels
 *
 * |param startLabelId[Start Label ID] Align captured input to the specified label ID.
 * An empty label ID disables this feature.
 * |default ""
 * |widget StringEntry()
 * |preview disable
 * |tab Labels
 *
 * |mode graphWidget
 * |factory /plotters/spectrogram(remoteEnv)
 * |setter setTitle(title)
 * |setter setDisplayRate(displayRate)
 * |setter setSampleRate(sampleRate)
 * |setter setCenterFrequency(centerFreq)
 * |setter setNumFFTBins(numBins)
 * |setter setWindowType(window, windowArgs)
 * |setter setFullScale(fullScale)
 * |setter setFFTMode(fftMode)
 * |setter setTimeSpan(timeSpan)
 * |setter setReferenceLevel(refLevel)
 * |setter setDynamicRange(dynRange)
 * |setter enableXAxis(enableXAxis)
 * |setter enableYAxis(enableYAxis)
 * |setter setColorMap(colorMap)
 * |setter setFreqLabelId(freqLabelId)
 * |setter setRateLabelId(rateLabelId)
 * |setter setStartLabelId(startLabelId)
 **********************************************************************/
class Spectrogram : public Pothos::Topology
{
public:
    static Topology *make(const Pothos::ProxyEnvironment::Sptr &remoteEnv)
    {
        return new Spectrogram(remoteEnv);
    }

    Spectrogram(const Pothos::ProxyEnvironment::Sptr &remoteEnv)
    {
        _display.reset(new SpectrogramDisplay());
        _display->setName("Display");

        auto registry = remoteEnv->findProxy("Pothos/BlockRegistry");
        _trigger = registry.call("/comms/wave_trigger");
        _trigger.call("setName", "Trigger");
        _trigger.call("setMode", "PERIODIC");

        //register calls in this topology
        this->registerCall(this, POTHOS_FCN_TUPLE(Spectrogram, setNumFFTBins));
        this->registerCall(this, POTHOS_FCN_TUPLE(Spectrogram, setFreqLabelId));
        this->registerCall(this, POTHOS_FCN_TUPLE(Spectrogram, setRateLabelId));
        this->registerCall(this, POTHOS_FCN_TUPLE(Spectrogram, setStartLabelId));

        //connect to internal display block
        this->connect(this, "setTitle", _display, "setTitle");
        this->connect(this, "setDisplayRate", _display, "setDisplayRate");
        this->connect(this, "setSampleRate", _display, "setSampleRate");
        this->connect(this, "setCenterFrequency", _display, "setCenterFrequency");
        this->connect(this, "setNumFFTBins", _display, "setNumFFTBins");
        this->connect(this, "setWindowType", _display, "setWindowType");
        this->connect(this, "setFullScale", _display, "setFullScale");
        this->connect(this, "setTimeSpan", _display, "setTimeSpan");
        this->connect(this, "setReferenceLevel", _display, "setReferenceLevel");
        this->connect(this, "setDynamicRange", _display, "setDynamicRange");
        this->connect(this, "enableXAxis", _display, "enableXAxis");
        this->connect(this, "enableYAxis", _display, "enableYAxis");
        this->connect(this, "setColorMap", _display, "setColorMap");
        this->connect(_display, "frequencySelected", this, "frequencySelected");
        this->connect(_display, "relativeFrequencySelected", this, "relativeFrequencySelected");

        //connect to the internal snooper block
        this->connect(_display, "updateRateChanged", _trigger, "setEventRate");
        this->connect(this, "setNumFFTBins", _trigger, "setNumPoints");

        //connect stream ports
        this->connect(this, 0, _trigger, 0);
        this->connect(_trigger, 0, _display, 0);
    }

    Pothos::Object opaqueCallMethod(const std::string &name, const Pothos::Object *inputArgs, const size_t numArgs) const
    {
        //calls that go to the topology
        try
        {
            return Pothos::Topology::opaqueCallMethod(name, inputArgs, numArgs);
        }
        catch (const Pothos::BlockCallNotFound &){}

        //forward everything else to display
        return _display->opaqueCallMethod(name, inputArgs, numArgs);
    }

    void setNumFFTBins(const size_t num)
    {
        _trigger.call("setNumPoints", num);
        _display->setNumFFTBins(num);
    }

    void setFreqLabelId(const std::string &id)
    {
        _display->setFreqLabelId(id);
        _freqLabelId = id;
        this->updateIdsList();
    }

    void setRateLabelId(const std::string &id)
    {
        _display->setRateLabelId(id);
        _rateLabelId = id;
        this->updateIdsList();
    }

    void setStartLabelId(const std::string &id)
    {
        _trigger.call("setLabelId", id);
        _trigger.call("setMode", id.empty()?"PERIODIC":"NORMAL");
    }

    void updateIdsList(void)
    {
        std::vector<std::string> ids;
        if (not _freqLabelId.empty()) ids.push_back(_freqLabelId);
        if (not _rateLabelId.empty()) ids.push_back(_rateLabelId);
        _trigger.call("setIdsList", ids);
    }

private:
    Pothos::Proxy _trigger;
    std::shared_ptr<SpectrogramDisplay> _display;
    std::string _freqLabelId, _rateLabelId;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::BlockRegistry registerSpectrogram(
    "/plotters/spectrogram", &Spectrogram::make);

static Pothos::BlockRegistry registerSpectrogramOldPath(
    "/widgets/spectrogram", &Spectrogram::make);
