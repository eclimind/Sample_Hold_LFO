/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ===============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <algorithm>
#include <cmath>
#include "SequenceProcessor.h"

int guiX = 800;
int guiY = 600; 

//==============================================================================
Sample_Hold_LFOAudioProcessorEditor::Sample_Hold_LFOAudioProcessorEditor (Sample_Hold_LFOAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    setSize (guiX, guiY);

    startTimerHz(30);

    addAndMakeVisible(floorSlider); {
        floorSlider.setName("Floor");
        floorSlider.setSliderStyle(juce::Slider::LinearVertical);
        floorSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 48, 20);
        floorSlider.setColour(juce::Slider::trackColourId, juce::Colours::lightgrey);
        floorSlider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
        floorSlider.setRange(0.0, 100.0, 1.0);
        floorSlider.setValue(0.0, juce::dontSendNotification);
    }

    addAndMakeVisible(ceilingSlider); {
        ceilingSlider.setName("Ceiling");
        ceilingSlider.setSliderStyle(juce::Slider::LinearVertical);
        ceilingSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 48, 20);
        ceilingSlider.setColour(juce::Slider::trackColourId, juce::Colours::lightgrey);
        ceilingSlider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
        ceilingSlider.setRange(0.0, 100.0, 1.0);
        ceilingSlider.setValue(100.0, juce::dontSendNotification);
    }

    addAndMakeVisible(lockFloorButton); {
        lockFloorButton.setClickingTogglesState(true);
        lockFloorButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        lockFloorButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        lockFloorButton.setColour(juce::TextButton::textColourOnId, juce::Colours::yellow);
        updateLockButtons(lockFloorButton, false);

        lockFloorButton.onClick = [this] {
            floorLocked = lockFloorButton.getToggleState();

            if (floorLocked)
                floorValue = floorSlider.getValue();

            updateLockButtons(lockFloorButton, floorLocked);
            updateConstraintControls();
        };

        floorSlider.onValueChange = [this] {
            if (updatingControls)
                return;

            if (floorLocked) {
                floorSlider.setValue(floorValue, juce::dontSendNotification);
                return;
            }

            updateConstraintControls();
        };
    }

    addAndMakeVisible(lockCeilingButton); {
        lockCeilingButton.setClickingTogglesState(true);
        lockCeilingButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        lockCeilingButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        lockCeilingButton.setColour(juce::TextButton::textColourOnId, juce::Colours::yellow);
        updateLockButtons(lockCeilingButton, false);

        lockCeilingButton.onClick = [this] {
            ceilingLocked = lockCeilingButton.getToggleState();

            if (ceilingLocked)
                ceilingValue = ceilingSlider.getValue();

            updateLockButtons(lockCeilingButton, ceilingLocked);
            updateConstraintControls();
        };

        ceilingSlider.onValueChange = [this] {
            if (updatingControls)
                return;

            if (ceilingLocked) {
                ceilingSlider.setValue(ceilingValue, juce::dontSendNotification);
                return;
            }

            updateConstraintControls();
        };

        updateConstraintControls();
    }

    addAndMakeVisible(newStartingSeedButton); {
        newStartingSeedButton.setColour(juce::TextButton::buttonColourId,juce::Colours::limegreen);
        newStartingSeedButton.onClick = [this] {
            audioProcessor.generateNewStartingSeed();
            repaint();
        };

        newStartingSeedButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    }

    addAndMakeVisible(smoothingSlider); {
        addAndMakeVisible(smoothingLabel);

        smoothingSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        smoothingSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        smoothingSlider.setRange(0.0, 1.0, 0.01);
        smoothingSlider.setValue(0.0);

        smoothingSlider.onValueChange = [this] {
            smoothingAmount = static_cast<float>(smoothingSlider.getValue());
            repaint();
        };

        smoothingLabel.setJustificationType(juce::Justification::centred);
    }

    addAndMakeVisible(speedRate); {
        speedRate.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        speedRate.setRange(0.1, 1000.0, 0.1);
        speedRate.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);

        speedRate.onValueChange = [this] {
            graphRate = static_cast<float>(speedRate.getValue());
            audioProcessor.setConstantRate(graphRate);
        };
    }

    addAndMakeVisible(axisSlider); {
        axisSlider.setName("Axis");
        axisSlider.setSliderStyle(juce::Slider::LinearVertical);
        axisSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        axisSlider.setColour(juce::Slider::trackColourId, juce::Colours::lightgrey);
        axisSlider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
        axisSlider.setRange(0.0, 1.0, 0.001);
        axisSlider.setValue(0.5, juce::dontSendNotification);
        axisSlider.onValueChange = [this] { repaint(); };
    }

    addAndMakeVisible(guiScale); {
        guiScale.addItem("25%", 1);
        guiScale.addItem("50%", 2);
        guiScale.addItem("75%", 3);
        guiScale.addItem("100%", 4);
        guiScale.addItem("125%", 5);
        guiScale.addItem("150%", 6);
        guiScale.addItem("175%", 7);
        guiScale.addItem("200%", 8);
        guiScale.setSelectedId(4);

        guiScale.onChange = [this] {
            switch (guiScale.getSelectedId()) {
                case 1:
                    guiX = 400;
                    guiY = 300;
                    break;
                case 2:
                    guiX = 600;
                    guiY = 450;
                    break;
                case 3:
                    guiX = 700;
                    guiY = 525;
                    break;
                case 4:
                    guiX = 800;
                    guiY = 600;
                    break;
                case 5:
                    guiX = 1000;
                    guiY = 750;
                    break;
                case 6:
                    guiX = 1200;
                    guiY = 900;
                    break;
                case 7:
                    guiX = 1400;
                    guiY = 1050;
                    break;
                case 8:
                    guiX = 1600;
                    guiY = 1200;
                    break;
            }

            setSize(guiX, guiY);
        };
    }

    addAndMakeVisible(graphBlockSize); {
        graphBlockSize.setColour(juce::ComboBox::backgroundColourId,juce::Colours::green);
        int itemID = 1;
        for (int blockSize = 4; blockSize <= 128; blockSize *= 2) {
            graphBlockSize.addItem(juce::String(blockSize), itemID);
            itemID++;
        }

        graphBlockSize.setSelectedId(5, juce::dontSendNotification);

        graphBlockSize.onChange = [this] {
            const int selectedID = graphBlockSize.getSelectedId();

            if (selectedID > 0) {
                const int blockSize = 4 * (1 << (selectedID - 1));
                audioProcessor.setSequenceDisplayCount(blockSize);
                repaint();
            }
        };
    }
}

Sample_Hold_LFOAudioProcessorEditor::~Sample_Hold_LFOAudioProcessorEditor()
{
}

//==============================================================================
namespace
{
    double catmullRom (
        const std::vector<int>& values,
        int i,
        double t)
    {
        const int lastIndex = static_cast<int> (values.size()) - 1;

        const double p0 = values[std::max (0, i - 1)];
        const double p1 = values[i];
        const double p2 = values[i + 1];
        const double p3 = values[std::min (lastIndex, i + 2)];

        const double t2 = t * t;
        const double t3 = t2 * t;

        return 0.5 * (
            2.0 * p1
            + (-p0 + p2) * t
            + (2.0 * p0 - 5.0 * p1 + 4.0 * p2 - p3) * t2
            + (-p0 + 3.0 * p1 - 3.0 * p2 + p3) * t3);
    }
}

juce::Rectangle<int> Sample_Hold_LFOAudioProcessorEditor::getGraphBounds() const {
    const int graphTop = 100;
    const int graphBottom = getHeight() - 55;
    const int graphLeft = 32;
    const int graphRight = getWidth() - 195;

    return juce::Rectangle<int>(
        graphLeft,
        graphTop,
        juce::jmax(0, graphRight - graphLeft),
        juce::jmax(0, graphBottom - graphTop)
    );
}

void Sample_Hold_LFOAudioProcessorEditor::lfoGraph(juce::Graphics& g) {
    const auto lfoValues = audioProcessor.getLfoValues();
    const bool bipolarMode = audioProcessor.isBipolarMode();
    juce::Path lfoPath;
    auto graphArea = getGraphBounds();

    if (graphArea.getWidth() < 2 || graphArea.getHeight() < 2)
        return;

    const float graphTop = static_cast<float>(graphArea.getY()-3);
    const float graphBottom = static_cast<float>(graphArea.getBottom() - 1);
    const float graphLeft = static_cast<float>(graphArea.getX());
    const float graphRight = static_cast<float>(graphArea.getRight() - 1);

    const float axisPosition = 1.0f - static_cast<float>(axisSlider.getValue());
    const float zeroYAxis = graphTop + axisPosition * (graphBottom - graphTop);

    const auto drawableWidth = graphRight - graphLeft;
    const auto drawableHeight = graphBottom - graphTop;

    const int sampleCount = static_cast<int>(lfoValues.size());

    const auto smoothnessAmount = juce::jlimit(0.0f, 1.0f, smoothingAmount);
    const auto speedRate = juce::jlimit(0.0f, 1.0f, graphRate);

    const auto toGraphX = [&](double index) {
        const auto fraction = sampleCount <= 1 ? 0.0 : index / static_cast<double>(sampleCount - 1);
        return graphLeft + static_cast<float>(fraction) * drawableWidth;
    };

    const auto toGraphY = [&](double value) {
        if (bipolarMode) {
            value = juce::jlimit(-100.0, 100.0, value);

            if (value >= 0.0)
                return zeroYAxis - static_cast<float>(value / 100.0) * (zeroYAxis - graphTop);

            return zeroYAxis + static_cast<float>(-value / 100.0) * (graphBottom - zeroYAxis);
        }
        else {
            value = juce::jlimit(0.0, 100.0, value);

            if (value >= 50.0)
                return zeroYAxis - static_cast<float>((value - 50.0) / 50.0) * (zeroYAxis - graphTop);

            return zeroYAxis + static_cast<float>((50.0 - value) / 50.0) * (graphBottom - zeroYAxis);
        }
    };

    if (!lfoValues.empty()) {
        if (smoothnessAmount <= 0.001f) {
            for (int i = 0; i < sampleCount; ++i) {
                const auto x = toGraphX(i);
                const auto y = toGraphY(lfoValues[i]);

                if (i == 0) {
                    lfoPath.startNewSubPath(x, y);
                }
                else {
                    const auto previousY = toGraphY(lfoValues[i - 1]);
                    lfoPath.lineTo(x, previousY);
                    lfoPath.lineTo(x, y);
                }
            }
        }
        else {
            constexpr int subdivisions = 16;

            const double p = 1.0 - smoothnessAmount;
            const double exponent = 1.0 + 63.0 * p * p;

            lfoPath.startNewSubPath(toGraphX(0), toGraphY(lfoValues[0]));

            for (int i = 0; i < sampleCount - 1; ++i) {
                const double A = lfoValues[i];
                const double B = lfoValues[i + 1];

                for (int step = 1; step <= subdivisions; ++step) {
                    const double t = static_cast<double>(step) / subdivisions;
                    const double u = std::pow(t, exponent);
                    const double s = u * u * (3.0 - 2.0 * u);
                    const double value = A + (B - A) * s;
                    const double samplePosition = i + t;

                    lfoPath.lineTo(toGraphX(samplePosition), toGraphY(value));
                }
            }
        }
    }

    g.setColour(juce::Colours::grey);
    g.drawHorizontalLine(juce::roundToInt(zeroYAxis), graphLeft, graphRight);
    g.drawVerticalLine(graphArea.getX(), graphTop, graphBottom);

    g.setColour(juce::Colours::lightgrey);

    if (bipolarMode) {
        g.drawText("+100", 5, graphArea.getY() - 10, 40, 20, juce::Justification::centredRight);
        g.drawText("0", 20, juce::roundToInt((graphTop + graphBottom) * 0.5f - 10), 25, 20, juce::Justification::centredRight);
        g.drawText("-100", 0, graphArea.getBottom() - 10, 45, 20, juce::Justification::centredRight);
    }
    else {
        g.drawText("100", 5, graphArea.getY() - 10, 40, 20, juce::Justification::centredRight);
        g.drawText("50", 20, juce::roundToInt((graphTop + graphBottom) * 0.5f - 10), 25, 20, juce::Justification::centredRight);
        g.drawText("0", 0, graphArea.getBottom() - 10, 45, 20, juce::Justification::centredRight);
    }

    g.drawText("0", graphArea.getX() - 5, graphArea.getBottom() + 5, 30, 20, juce::Justification::centredLeft);

    if (sampleCount > 0) {
        g.drawText(juce::String(sampleCount - 1), graphArea.getRight() - 30, graphArea.getBottom() + 5, 35, 20, juce::Justification::centredRight);
    }

    if (!lfoValues.empty()) {
        g.saveState();
        g.reduceClipRegion(graphArea);
        g.setColour(juce::Colours::green);
        g.strokePath(lfoPath, juce::PathStrokeType(2.0f));
        g.restoreState();
    }
}

void Sample_Hold_LFOAudioProcessorEditor::timerCallback()
{
    repaint();
}

void Sample_Hold_LFOAudioProcessorEditor::lfoConstraints(juce::Graphics& g)
{
    const auto graphArea = getGraphBounds();

    const float scaleX = static_cast<float>(getWidth()) / 800.0f;
    const float scaleY = static_cast<float>(getHeight()) / 600.0f;
    const float scale = juce::jmin(scaleX, scaleY);

    const int constraintWidth = juce::roundToInt(100 * scale);
    const int constraintGap = juce::roundToInt(12 * scale);

    auto constraintArea = juce::Rectangle<int>(
        graphArea.getRight() + constraintGap,
        graphArea.getY(),
        constraintWidth,
        graphArea.getHeight()
    );

    g.setColour(juce::Colours::darkgrey);
    g.drawRect(constraintArea, 1);

    const auto lineY = [&constraintArea](double value) {
        const auto fraction = value / 100.0;
        return constraintArea.getBottom() - 1 - juce::roundToInt(fraction * (constraintArea.getHeight() - 2));
    };

    const auto getThumbY = [](juce::Slider& slider) {
        const auto sliderBounds = slider.getBounds();
        const auto proportion = slider.valueToProportionOfLength(slider.getValue());

        const auto trackTop = sliderBounds.getY() + 10;
        const auto trackBottom = sliderBounds.getBottom() - 30;

        return trackBottom - juce::roundToInt(proportion * (trackBottom - trackTop));
    };


    const auto floorY = getThumbY(floorSlider);
    const auto ceilingY = getThumbY(ceilingSlider);

    g.setColour(floorLocked ? juce::Colours::yellow : juce::Colours::lightgrey);
    g.drawHorizontalLine(floorY,
        static_cast<float>(constraintArea.getX() + 3),
        static_cast<float>(constraintArea.getRight() - 3));

    g.setColour(ceilingLocked ? juce::Colours::yellow : juce::Colours::lightgrey);
    g.drawHorizontalLine(ceilingY,
        static_cast<float>(constraintArea.getX() + 3),
        static_cast<float>(constraintArea.getRight() - 3));
}

void Sample_Hold_LFOAudioProcessorEditor::updateLockButtons(juce::TextButton& button, bool isLocked)
{
    button.setButtonText(isLocked ? "L" : "U");
}

void Sample_Hold_LFOAudioProcessorEditor::updateConstraintControls()
{
    if (updatingControls)
        return;

    updatingControls = true;

    double floor = floorSlider.getValue();
    double ceiling = ceilingSlider.getValue();

    floor = juce::jlimit(0.0, 100.0, floor);
    ceiling = juce::jlimit(0.0, 100.0, ceiling);

    if (floor > ceiling) {
        if (floorSlider.isMouseButtonDown() && !floorLocked)
            floor = ceiling;
        else
            ceiling = floor;
    }

    if (floorLocked)
        floor = floorValue;

    if (ceilingLocked)
        ceiling = ceilingValue;

    if (floor > ceiling) {
        if (!floorLocked)
            floor = ceiling;
        else if (!ceilingLocked)
            ceiling = floor;
        else {
            floor = juce::jmin(floor, ceiling);
        }
    }

    floorSlider.setValue(floor, juce::dontSendNotification);
    ceilingSlider.setValue(ceiling, juce::dontSendNotification);

    audioProcessor.setOutputConstraints(false, juce::roundToInt(floor), juce::roundToInt(ceiling));

    repaint();

    updatingControls = false;
}

void Sample_Hold_LFOAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    auto panel = getLocalBounds().reduced(18, 15);
    panel.removeFromTop(50);

    g.setColour(juce::Colour(0xff303d43));
    g.fillRoundedRectangle(panel.toFloat(), 8.0f);

    g.setColour(juce::Colour(0xff46545a));
    g.drawRoundedRectangle(panel.toFloat(), 8.0f, 1.0f);

    lfoGraph(g);
    lfoConstraints(g);
}

void Sample_Hold_LFOAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    const float scaleX = static_cast<float>(getWidth()) / 800.0f;
    const float scaleY = static_cast<float>(getHeight()) / 600.0f;
    const float scale = juce::jmin(scaleX, scaleY);

    const auto scaled = [scale](int value) {
        return juce::roundToInt(value * scale);
    };

    auto guiArea = getLocalBounds();

    auto panel = getLocalBounds().reduced(18, 15);
    panel.removeFromTop(50);
    
    const int panelPadding = scaled(14);
    auto content = panel.reduced(panelPadding);

    auto graphArea = getGraphBounds();
    const int constraintWidth = scaled(100);
    const int constraintGap = scaled(12);
    auto constraintArea = juce::Rectangle<int>(
        graphArea.getRight() + constraintGap,
        graphArea.getY(),
        constraintWidth,
        graphArea.getHeight()
    );

    const int lockSize = scaled(22);
    const int sliderGap = scaled(8);
    const int sliderWidth = (constraintArea.getWidth() - sliderGap) / 2;
    const int textboxHeight = scaled(20);
    const int lockY = constraintArea.getY();
    const int sliderTop = lockY + lockSize + scaled(5);
    const int sliderBottom = constraintArea.getBottom() - textboxHeight;
    const int sliderHeight = juce::jmax(1, sliderBottom - sliderTop);

    lockFloorButton.setBounds(constraintArea.getX(), lockY, lockSize, lockSize);
    floorSlider.setBounds(constraintArea.getX(), sliderTop, sliderWidth, sliderHeight);

    lockCeilingButton.setBounds(constraintArea.getX() + sliderWidth + sliderGap, lockY, lockSize, lockSize);
    ceilingSlider.setBounds(constraintArea.getX() + sliderWidth + sliderGap, sliderTop, sliderWidth, sliderHeight);
    
    const int controlSize = scaled(50)*(1.5);
    const int controlGap = controlSize / 4;
    const int controlY = scaled(10);
    const int controlStartX = scaled(20);

    newStartingSeedButton.setBounds(controlStartX,controlY,controlSize,controlSize / (1.5));

    smoothingSlider.setBounds(newStartingSeedButton.getRight() + controlGap,controlY,controlSize,controlSize);

    smoothingLabel.setBounds(smoothingSlider.getX(),smoothingSlider.getY() - scaled(18),controlSize,scaled(18));

    speedRate.setBounds(smoothingSlider.getRight() + controlGap,controlY,controlSize,controlSize);

    auto axisArea = content.removeFromRight(scaled(28));
    content.removeFromRight(scaled(8));
    axisSlider.setBounds(axisArea.getX() + scaled(4), content.getY() + scaled(8), scaled(20), content.getHeight() - scaled(16));

    const int comboWidth = scaled(60)*(1.55);
    const int comboHeight = scaled(30);
    const int comboMargin = scaled(12);

    guiScale.setBounds(getWidth() - comboWidth - comboMargin, newStartingSeedButton.getY(), comboWidth, comboHeight);
    graphBlockSize.setBounds(guiScale.getX() - sliderGap - comboWidth, newStartingSeedButton.getY(), comboWidth, comboHeight);
    
}
