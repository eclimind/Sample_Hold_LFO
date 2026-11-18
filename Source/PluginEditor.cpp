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

juce::Rectangle<int> Sample_Hold_LFOAudioProcessorEditor::getGridArea(int column, int row, int columnSpan, int rowSpan) const {
    const auto topLeft = guiGrid[row][column];
    const auto bottomRight = guiGrid[row + rowSpan - 1][column + columnSpan - 1];

    return {
        topLeft.getX(),
        topLeft.getY(),
        bottomRight.getRight() - topLeft.getX(),
        bottomRight.getBottom() - topLeft.getY()
    };
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
    return getGridArea(6, 20, 115, 89);
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
        g.drawText("+100", getGridArea(0, 18, 9, 4), juce::Justification::centredRight);
        g.drawText("0", getGridArea(0, 62, 9, 4), juce::Justification::centredRight);
        g.drawText("-100", getGridArea(0, 107, 9, 4), juce::Justification::centredRight);
    }
    else {
        g.drawText("100", getGridArea(0, 18, 9, 4), juce::Justification::centredRight);
        g.drawText("50", getGridArea(0, 62, 9, 4), juce::Justification::centredRight);
        g.drawText("0", getGridArea(0, 107, 9, 4), juce::Justification::centredRight);
    }
    g.drawText("0", getGridArea(5, 110, 6, 4), juce::Justification::centredLeft);
    if (sampleCount > 0) {
        g.drawText(juce::String(sampleCount - 1), getGridArea(115, 110, 7, 4), juce::Justification::centredRight);
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
    const auto constraintArea = getGridArea(123, 20, 20, 89);

    g.setColour(juce::Colours::darkgrey);
    g.drawRect(constraintArea, 1);

    const auto lineY = [&constraintArea](double value) {
        const auto fraction = value / 100.0;
        return constraintArea.getBottom() - 1 - juce::roundToInt(fraction * (constraintArea.getHeight() - 2));
    };

    const auto getThumbY = [this](juce::Slider& slider) {
        const auto sliderBounds = slider.getBounds();
        const auto proportion = slider.valueToProportionOfLength(slider.getValue());

        const auto trackTop = sliderBounds.getY() + guiGrid[0][0].getHeight() * 2;
        const auto trackBottom = sliderBounds.getBottom() - guiGrid[0][0].getHeight() * 6;

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

    auto panel = getGridArea(4, 13, 152, 104);

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

    const float cellWidth = static_cast<float>(getWidth()) / GRID_COLUMNS;
    const float cellHeight = static_cast<float>(getHeight()) / GRID_ROWS;

    for (int row = 0; row < GRID_ROWS; row++) {
        for (int column = 0; column < GRID_COLUMNS; column++) {
            guiGrid[row][column] = {
                juce::roundToInt(column * cellWidth),
                juce::roundToInt(row * cellHeight),
                juce::roundToInt(cellWidth),
                juce::roundToInt(cellHeight)
            };
        }
    }

    const auto scaled = [scale](int value) {
        return juce::roundToInt(value * scale);
    };

    auto guiArea = getLocalBounds();

    auto panel = getLocalBounds().reduced(18, 15);
    panel.removeFromTop(50);
    
    const int panelPadding = scaled(14);
    auto content = panel.reduced(panelPadding);

    auto graphArea = getGraphBounds();
    auto constraintArea = getGridArea(123, 20, 20, 89).translated(scaled(2), 0);

    lockFloorButton.setBounds(getGridArea(123, 20, 5, 5).withSize(scaled(22), scaled(22)).translated(scaled(2), 0));
    floorSlider.setBounds(getGridArea(123, 25, 9, 80).withSize(scaled(46), scaled(398)).translated(scaled(2), scaled(2)));

    lockCeilingButton.setBounds(getGridArea(134, 20, 5, 5).withSize(scaled(22), scaled(22)).translated(scaled(1), 0));
    ceilingSlider.setBounds(getGridArea(134, 25, 9, 80).withSize(scaled(46), scaled(398)).translated(scaled(1), scaled(2)));


    newStartingSeedButton.setBounds(getGridArea(4, 2, 15, 10));
    smoothingSlider.setBounds(getGridArea(23, 2, 15, 15));
    smoothingLabel.setBounds(getGridArea(23, 0, 15, 4).translated(0, -8));
    speedRate.setBounds(getGridArea(41, 2, 15, 15));

    axisSlider.setBounds(getGridArea(149, 17, 4, 95).withHeight(scaled(476)).translated(-scaled(1), scaled(2)));

    guiScale.setBounds(getGridArea(139, 2, 19, 6));
    graphBlockSize.setBounds(getGridArea(119, 2, 19, 6));

}
