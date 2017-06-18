//
// This software is released under the MIT licence
//
// Copyright (c) 2013-2017 Anders Langlands
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
//      The above copyright notice and this permission notice shall be included
//      in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//

// This code is ported from alshaders original C++ implementation:
// https://bitbucket.org/anderslanglands/alshaders
// with some modifications by the appleseed team:
// https://github.com/appleseedhq/appleseed/

float bias(float x, float b) { return x / ((1 / b - 2) * (1 - x) + 1); }

float biasandgain(float x, float b, float g) {
    x = bias(x, b);
    if (x < 0.5) {
        return x / ((1 / g - 2) * (1 - 2 * x) + 1);
    } else {
        return ((1 / g - 2) * (1 - 2 * x) - x) /
            ((1 / g - 2) * (1 - 2 * x) - 1);
    }
}

float contrast(float x, float contrast, float pivot) {
    if (contrast == 1) {
        return x;
    }
    return (x - pivot) * contrast + pivot;
}

color contrast(color x, float contrast, float pivot) {
    if (contrast == 1) {
        return x;
    }
    return color((x[0] - pivot) * contrast + pivot,
            (x[1] - pivot) * contrast + pivot,
            (x[2] - pivot) * contrast + pivot);
}

float al_rmp_do_remap(float input, float imn, float imx, float c, float cp,
        float b, float g, float omn, float omx, int rc, int ex,
        float cmn, float cmx) {
    float x = (input - imn) / (imx - imn);
    x = contrast(x, c, cp);
    x = biasandgain(x, b, g);
    x = mix(omn, omx, x);
    if (rc) {
        x = clamp(x, cmn, cmx);
        if (ex) {
            x = (x - cmn) / (cmx - cmn);
        }
    }
    return x;
}

#define AL_REMAP_FLOAT_DECLARE(PARAM_PAGE)                                     \
    float rmp_input_min = 0 [[                                                 \
string label = "Input min",                                            \
string page = #PARAM_PAGE,   \
string help = "Set this to the minimum expected value of the input "   \
"to remap the input to start at 0 before applying the "  \
"rest of the remap functions."                           \
    ]],                                                                        \
float rmp_input_max = 1 [[                                           \
string label = "Input max",                                      \
string page = #PARAM_PAGE,   \
string help = "Set this to the maximum expected value of the "   \
"input to remap the input to end at 1 before "     \
"applying the rest of the remap functions."        \
]],                                                                  \
float rmp_contrast = 1 [[                                            \
string label = "Contrast",                                       \
string page = #PARAM_PAGE,   \
string help = "Values greater than 1 increase contrast about "   \
"the Pivot, values less than 1 reduce it.",        \
float min = 0                                                    \
]],                                                                  \
float rmp_contrast_pivot = 0.5 [[                                    \
string label = "Pivot",                                          \
string page = #PARAM_PAGE,   \
string help = "The 'center point' around which the Contrast is " \
"performed."                                       \
]],                                                                  \
float rmp_bias = 0.5 [[                                              \
string label = "Bias",                                           \
string page = #PARAM_PAGE,   \
string help = "Values greater than 0.5 compress the function "   \
"towards 1 while values less than 0.5 compress "   \
"it torwards 0",                                   \
float min = 0, float max = 1                                     \
]],                                                                  \
float rmp_gain = 0.5 [[                                              \
string label = "Gain",                                           \
string page = #PARAM_PAGE,   \
string help = "Values less then 0.5 turn the input function "    \
"into an s-curve, while values greater than 0.5 "  \
"give a 'horizontal s-curve'",                     \
float min = 0, float max = 1                                     \
]],                                                                  \
float rmp_output_min = 0 [[                                          \
string label = "Output min",                                     \
string page = #PARAM_PAGE,   \
string help =                                                    \
"Set this to the minimum value of the desired output "       \
"range. The output will be interpolated between this "       \
"and Output max."                                            \
]],                                                                  \
float rmp_output_max = 1 [[                                          \
string label = "Output max",                                     \
string page = #PARAM_PAGE,   \
string help = "Set this to the maximum value of "                \
"the desired output range. The "                   \
"output will be interpolated "                     \
"between Output min and this."                     \
]],                                                                  \
int rmp_clamp = 1 [[                                                 \
string label = "Clamp",                                          \
string page = #PARAM_PAGE,   \
string help =                                                    \
"Enable this to clamp the output to [Clamp min, Clamp "      \
"max]",                                                      \
string widget = "checkBox"                                       \
]],                                                                  \
float rmp_clamp_min = 0 [[                                           \
string page = #PARAM_PAGE,   \
string label = "Clamp min",                                      \
string help = "Output values smaller than this value will be "   \
"clipped to this value"                            \
]],                                                                  \
float rmp_clamp_max = 1 [[                                           \
string label = "Clamp max",                                      \
string page = #PARAM_PAGE,   \
string help = "Output values larger than this value will be "    \
"clipped to this value."                           \
]],                                                                  \
int rmp_expand = 0 [[                                                \
string label = "Expand",                                         \
string page = #PARAM_PAGE,   \
string help =                                                    \
"Enable this to remapt the post-clamped range to [0,1]",     \
string widget = "checkBox"                                       \
]]

#define AL_REMAP_FLOAT(x)                                                      \
    al_rmp_do_remap((x), rmp_input_min, rmp_input_max, rmp_contrast,           \
            rmp_contrast_pivot, rmp_bias, rmp_gain, rmp_output_min,    \
            rmp_output_max, rmp_clamp, rmp_expand, rmp_clamp_min,      \
            rmp_clamp_max)
