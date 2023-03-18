/*
    ForceSensitiveResistor.h
    Date: 18 March 2023
    Author: Berk Yaşar Yavuz
    Mail: berkyavuz1997@gmail.com
*/

#pragma once
#include <Arduino.h>

/*
    Class definition
*/
class ForceSensitiveResistor
{
private:
    unsigned int _steps = 0;

    uint8_t _pin;
    uint16_t _adcThreshold;
    uint32_t _minStepDurationThreshold;
    uint32_t _minStepIntervalThreshold;

    unsigned long _counter = 0;
    unsigned long _lastStepTime = 0;

    void _countSteps();

    // Force related functions
    unsigned long _getForceAppliedDuration();
    bool _forceApplied();

    // Counter related functions
    void _startCounter();
    void _resetCounter();
    bool _isCounterRunning();
    unsigned long _counterDuration();

public:
    ForceSensitiveResistor(const uint8_t pin, const uint16_t adcThreshold = 3900, const uint32_t minStepDurationThreshold = 300, const uint32_t minStepIntervalThreshold = 1000);

    // This function must be called frequently for this class to work properly. You can place it in your main loop, or put it in a freertos task.
    void run();

    // Returns the number of steps counted since the last reset
    unsigned int getSteps();

    // Resets the number of steps counted
    void resetSteps();
};

/*
    Constructor
*/
ForceSensitiveResistor::ForceSensitiveResistor(const uint8_t pin, const uint16_t adcThreshold, const uint32_t minStepDurationThreshold, const uint32_t minStepIntervalThreshold) : _pin(pin), _adcThreshold(adcThreshold), _minStepDurationThreshold(minStepDurationThreshold), _minStepIntervalThreshold(minStepIntervalThreshold)
{
    pinMode(_pin, INPUT);
    _resetCounter();
    resetSteps();
}

/*
    Public Functions
*/

void ForceSensitiveResistor::run()
{
    _countSteps();
}

unsigned int ForceSensitiveResistor::getSteps()
{
    return _steps;
}

void ForceSensitiveResistor::resetSteps()
{
    _steps = 0;
}

/*
    Private Functions
*/
void ForceSensitiveResistor::_countSteps()
{
    if ((_getForceAppliedDuration() > _minStepDurationThreshold) && (millis() - _lastStepTime > _minStepIntervalThreshold))
    {
        _lastStepTime = millis();
        _steps++;
    }
}

unsigned long ForceSensitiveResistor::_getForceAppliedDuration()
{
    unsigned long forceAppliedDuration = 0;

    if (_forceApplied())
    {
        if (!_isCounterRunning())
            _startCounter();
    }
    else
    {
        forceAppliedDuration = _counterDuration();
        _resetCounter();
    }

    return forceAppliedDuration;
}

bool ForceSensitiveResistor::_forceApplied()
{
    return analogRead(_pin) > _adcThreshold;
}

void ForceSensitiveResistor::_startCounter()
{
    _counter = millis();
}

void ForceSensitiveResistor::_resetCounter()
{
    _counter = 0;
}

bool ForceSensitiveResistor::_isCounterRunning()
{
    return _counter > 0;
}

unsigned long ForceSensitiveResistor::_counterDuration()
{
    return _isCounterRunning() ? millis() - _counter : 0;
}

/*
    main.cpp
    Date: 18 March 2023
    Author: Berk Yaşar Yavuz
    Mail: berkyavuz1997@gmail.com
*/