/*
    src/example1.cpp -- C++ version of an example application that shows
    how to use the various widget classes. For a Python implementation, see
    '../python/example1.py'.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/opengl.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/icons.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/colorpicker.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>
#include <nanogui/texture.h>
#include <nanogui/shader.h>
#include <nanogui/renderpass.h>
#include <iostream>
#include <memory>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"

#include <streambuf>
#include <fstream>
#include "../ImGuiColorTextEdit/TextEditor.h"


using namespace nanogui;


static const char* fileToEdit = "../ImGuiColorTextEdit/TextEditor.cpp";
TextEditor editor;
auto lang = TextEditor::LanguageDefinition::CPlusPlus();


class ExampleApplication : public Screen {
public:
    ExampleApplication() : Screen(Vector2i(1024, 768), "NanoGUI Test") {
        inc_ref();
        Window *window = new Window(this, "Button demo");
        window->set_position(Vector2i(15, 15));
        window->set_layout(new GroupLayout());

        /* No need to store a pointer, the data structure will be automatically
           freed when the parent window is deleted */
        new Label(window, "Push buttons", "sans-bold");

        Button *b = new Button(window, "Plain button");
        b->set_callback([] { std::cout << "pushed!" << std::endl; });
        b->set_tooltip("short tooltip");

        /* Alternative construction notation using variadic template */
        b = window->add<Button>("Styled", FA_ROCKET);
        b->set_background_color(Color(0, 0, 255, 25));
        b->set_callback([] { std::cout << "pushed!" << std::endl; });
        b->set_tooltip("This button has a fairly long tooltip. It is so long, in "
                "fact, that the shown text will span several lines.");

        new Label(window, "Toggle buttons", "sans-bold");
        b = new Button(window, "Toggle me");
        b->set_flags(Button::ToggleButton);
        b->set_change_callback([](bool state) { std::cout << "Toggle button state: " << state << std::endl; });

        new Label(window, "Radio buttons", "sans-bold");
        b = new Button(window, "Radio button 1");
        b->set_flags(Button::RadioButton);
        b = new Button(window, "Radio button 2");
        b->set_flags(Button::RadioButton);

        new Label(window, "A tool palette", "sans-bold");
        Widget *tools = new Widget(window);
        tools->set_layout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 6));

        b = new ToolButton(tools, FA_CLOUD);
        b = new ToolButton(tools, FA_FAST_FORWARD);
        b = new ToolButton(tools, FA_COMPASS);
        b = new ToolButton(tools, FA_UTENSILS);

        new Label(window, "Popup buttons", "sans-bold");
        PopupButton *popup_btn = new PopupButton(window, "Popup", FA_FLASK);
        Popup *popup = popup_btn->popup();
        popup->set_layout(new GroupLayout());
        new Label(popup, "Arbitrary widgets can be placed here");
        new CheckBox(popup, "A check box");
        // popup right
        popup_btn = new PopupButton(popup, "Recursive popup", FA_CHART_PIE);
        Popup *popup_right = popup_btn->popup();
        popup_right->set_layout(new GroupLayout());
        new CheckBox(popup_right, "Another check box");
        // popup left
        popup_btn = new PopupButton(popup, "Recursive popup", FA_DNA);
        popup_btn->set_side(Popup::Side::Left);
        Popup *popup_left = popup_btn->popup();
        popup_left->set_layout(new GroupLayout());
        new CheckBox(popup_left, "Another check box");

        window = new Window(this, "Basic widgets");
        window->set_position(Vector2i(200, 15));
        window->set_layout(new GroupLayout());

        new Label(window, "Message dialog", "sans-bold");
        tools = new Widget(window);
        tools->set_layout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 6));
        b = new Button(tools, "Info");
        b->set_callback([&] {
            auto dlg = new MessageDialog(this, MessageDialog::Type::Information, "Title", "This is an information message");
            dlg->set_callback([](int result) { std::cout << "Dialog result: " << result << std::endl; });
        });
        b = new Button(tools, "Warn");
        b->set_callback([&] {
            auto dlg = new MessageDialog(this, MessageDialog::Type::Warning, "Title", "This is a warning message");
            dlg->set_callback([](int result) { std::cout << "Dialog result: " << result << std::endl; });
        });
        b = new Button(tools, "Ask");
        b->set_callback([&] {
            auto dlg = new MessageDialog(this, MessageDialog::Type::Warning, "Title", "This is a question message", "Yes", "No", true);
            dlg->set_callback([](int result) { std::cout << "Dialog result: " << result << std::endl; });
        });

#if defined(_WIN32)
        /// Executable is in the Debug/Release/.. subdirectory
        std::string resources_folder_path("../resources/icons");
#else
        std::string resources_folder_path("./icons");
#endif
        std::vector<std::pair<int, std::string>> icons;

#if !defined(EMSCRIPTEN)
        try {
            icons = load_image_directory(m_nvg_context, resources_folder_path);
        } catch (const std::exception &e) {
            std::cerr << "Warning: " << e.what() << std::endl;
        }
#endif

        new Label(window, "Image panel & scroll panel", "sans-bold");
        PopupButton *image_panel_btn = new PopupButton(window, "Image Panel");
        image_panel_btn->set_icon(FA_IMAGES);
        popup = image_panel_btn->popup();
        VScrollPanel *vscroll = new VScrollPanel(popup);
        ImagePanel *img_panel = new ImagePanel(vscroll);
        img_panel->set_images(icons);
        popup->set_fixed_size(Vector2i(245, 150));

        auto image_window = new Window(this, "Selected image");
        image_window->set_position(Vector2i(710, 15));
        image_window->set_layout(new GroupLayout(3));

        // Create a Texture instance for each object
        for (auto& icon : icons) {
            Vector2i size;
            int n = 0;
            ImageHolder texture_data(
                stbi_load((icon.second + ".png").c_str(), &size.x(), &size.y(), &n, 0),
                stbi_image_free);
            assert(n == 4);

            Texture *tex = new Texture(
                Texture::PixelFormat::RGBA,
                Texture::ComponentFormat::UInt8,
                size,
                Texture::InterpolationMode::Trilinear,
                Texture::InterpolationMode::Nearest);

            tex->upload(texture_data.get());

            m_images.emplace_back(tex, std::move(texture_data));
        }

        ImageView *image_view = new ImageView(image_window);
        if (!m_images.empty())
            image_view->set_image(m_images[0].first);
        image_view->center();
        m_current_image = 0;

        img_panel->set_callback([this, image_view](int i) {
            std::cout << "Selected item " << i << std::endl;
            image_view->set_image(m_images[i].first);
            m_current_image = i;
        });

        image_view->set_pixel_callback(
            [this](const Vector2i& index, char **out, size_t size) {
                const Texture *texture = m_images[m_current_image].first.get();
                uint8_t *data = m_images[m_current_image].second.get();
                for (int ch = 0; ch < 4; ++ch) {
                    uint8_t value = data[(index.x() + index.y() * texture->size().x())*4 + ch];
                    snprintf(out[ch], size, "%i", (int) value);
                }
            }
        );

        new Label(window, "File dialog", "sans-bold");
        tools = new Widget(window);
        tools->set_layout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 6));
        b = new Button(tools, "Open");
        b->set_callback([&] {
            std::cout << "File dialog result: " << file_dialog(
                    { {"png", "Portable Network Graphics"}, {"txt", "Text file"} }, false) << std::endl;
        });
        b = new Button(tools, "Save");
        b->set_callback([&] {
            std::cout << "File dialog result: " << file_dialog(
                    { {"png", "Portable Network Graphics"}, {"txt", "Text file"} }, true) << std::endl;
        });

        new Label(window, "Combo box", "sans-bold");
        new ComboBox(window, { "Combo box item 1", "Combo box item 2", "Combo box item 3"});
        new Label(window, "Check box", "sans-bold");
        CheckBox *cb = new CheckBox(window, "Flag 1",
            [](bool state) { std::cout << "Check box 1 state: " << state << std::endl; }
        );
        cb->set_checked(true);
        cb = new CheckBox(window, "Flag 2",
            [](bool state) { std::cout << "Check box 2 state: " << state << std::endl; }
        );
        new Label(window, "Progress bar", "sans-bold");
        m_progress = new ProgressBar(window);

        new Label(window, "Slider and text box", "sans-bold");

        Widget *panel = new Widget(window);
        panel->set_layout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 20));

        Slider *slider = new Slider(panel);
        slider->set_value(0.5f);
        slider->set_fixed_width(80);

        TextBox *text_box = new TextBox(panel);
        text_box->set_fixed_size(Vector2i(60, 25));
        text_box->set_value("50");
        text_box->set_units("%");
        slider->set_callback([text_box](float value) {
            text_box->set_value(std::to_string((int) (value * 100)));
        });
        slider->set_final_callback([&](float value) {
            std::cout << "Final slider value: " << (int) (value * 100) << std::endl;
        });
        text_box->set_fixed_size(Vector2i(60,25));
        text_box->set_font_size(20);
        text_box->set_alignment(TextBox::Alignment::Right);

        window = new Window(this, "Misc. widgets");
        window->set_position(Vector2i(425,15));
        window->set_layout(new GroupLayout());

        TabWidget* tab_widget = window->add<TabWidget>();

        Widget* layer = new Widget(tab_widget);
        layer->set_layout(new GroupLayout());
        tab_widget->append_tab("Color Wheel", layer);

        // Use overloaded variadic add to fill the tab widget with Different tabs.
        layer->add<Label>("Color wheel widget", "sans-bold");
        layer->add<ColorWheel>();

        layer = new Widget(tab_widget);
        layer->set_layout(new GroupLayout());
        tab_widget->append_tab("Function Graph", layer);

        layer->add<Label>("Function graph widget", "sans-bold");

        Graph *graph = layer->add<Graph>("Some Function");

        graph->set_header("E = 2.35e-3");
        graph->set_footer("Iteration 89");
        std::vector<float> &func = graph->values();
        func.resize(100);
        for (int i = 0; i < 100; ++i)
            func[i] = 0.5f * (0.5f * std::sin(i / 10.f) +
                              0.5f * std::cos(i / 23.f) + 1);

        // Dummy tab used to represent the last tab button.
        int plus_id = tab_widget->append_tab("+", new Widget(tab_widget));

        // A simple counter.
        int counter = 1;
        tab_widget->set_callback([tab_widget, this, counter, plus_id] (int id) mutable {
            if (id == plus_id) {
                // When the "+" tab has been clicked, simply add a new tab.
                std::string tab_name = "Dynamic " + std::to_string(counter);
                Widget* layer_dyn = new Widget(tab_widget);
                int new_id = tab_widget->insert_tab(tab_widget->tab_count() - 1,
                                                    tab_name, layer_dyn);
                layer_dyn->set_layout(new GroupLayout());
                layer_dyn->add<Label>("Function graph widget", "sans-bold");
                Graph *graph_dyn = layer_dyn->add<Graph>("Dynamic function");

                graph_dyn->set_header("E = 2.35e-3");
                graph_dyn->set_footer("Iteration " + std::to_string(new_id*counter));
                std::vector<float> &func_dyn = graph_dyn->values();
                func_dyn.resize(100);
                for (int i = 0; i < 100; ++i)
                    func_dyn[i] = 0.5f *
                        std::abs((0.5f * std::sin(i / 10.f + counter) +
                                  0.5f * std::cos(i / 23.f + 1 + counter)));
                ++counter;
                tab_widget->set_selected_id(new_id);

                // We must invoke the layout manager after adding tabs dynamically
                perform_layout();
            }
        });

        // A button to go back to the first tab and scroll the window.
        panel = window->add<Widget>();
        panel->add<Label>("Jump to tab: ");
        panel->set_layout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 6));

        auto ib = panel->add<IntBox<int>>();
        ib->set_editable(true);

        b = panel->add<Button>("", FA_FORWARD);
        b->set_fixed_size(Vector2i(22, 22));
        ib->set_fixed_height(22);
        b->set_callback([tab_widget, ib] {
            int value = ib->value();
            if (value >= 0 && value < tab_widget->tab_count())
                tab_widget->set_selected_index(value);
        });

        window = new Window(this, "Grid of small widgets");
        window->set_position(Vector2i(425, 300));
        GridLayout *layout =
            new GridLayout(Orientation::Horizontal, 2,
                           Alignment::Middle, 15, 5);
        layout->set_col_alignment(
            { Alignment::Maximum, Alignment::Fill });
        layout->set_spacing(0, 10);
        window->set_layout(layout);

        /* FP widget */ {
            new Label(window, "Floating point :", "sans-bold");
            text_box = new TextBox(window);
            text_box->set_editable(true);
            text_box->set_fixed_size(Vector2i(100, 20));
            text_box->set_value("50");
            text_box->set_units("GiB");
            text_box->set_default_value("0.0");
            text_box->set_font_size(16);
            text_box->set_format("[-]?[0-9]*\\.?[0-9]+");
        }

        /* Positive integer widget */ {
            new Label(window, "Positive integer :", "sans-bold");
            auto int_box = new IntBox<int>(window);
            int_box->set_editable(true);
            int_box->set_fixed_size(Vector2i(100, 20));
            int_box->set_value(50);
            int_box->set_units("Mhz");
            int_box->set_default_value("0");
            int_box->set_font_size(16);
            int_box->set_format("[1-9][0-9]*");
            int_box->set_spinnable(true);
            int_box->set_min_value(1);
            int_box->set_value_increment(2);
        }

        /* Checkbox widget */ {
            new Label(window, "Checkbox :", "sans-bold");

            cb = new CheckBox(window, "Check me");
            cb->set_font_size(16);
            cb->set_checked(true);
        }

        new Label(window, "Combo box :", "sans-bold");
        ComboBox *cobo =
            new ComboBox(window, { "Item 1", "Item 2", "Item 3" });
        cobo->set_font_size(16);
        cobo->set_fixed_size(Vector2i(100,20));

        new Label(window, "Color picker :", "sans-bold");
        auto cp = new ColorPicker(window, {255, 120, 0, 255});
        cp->set_fixed_size({100, 20});
        cp->set_final_callback([](const Color &c) {
            std::cout << "ColorPicker final callback: ["
                      << c.r() << ", "
                      << c.g() << ", "
                      << c.b() << ", "
                      << c.w() << "]" << std::endl;
        });
        // setup a fast callback for the color picker widget on a new window
        // for demonstrative purposes
        window = new Window(this, "Color Picker Fast Callback");
        layout = new GridLayout(Orientation::Horizontal, 2,
                                 Alignment::Middle, 15, 5);
        layout->set_col_alignment(
            { Alignment::Maximum, Alignment::Fill });
        layout->set_spacing(0, 10);
        window->set_layout(layout);
        window->set_position(Vector2i(425, 500));
        new Label(window, "Combined: ");
        b = new Button(window, "ColorWheel", FA_INFINITY);
        new Label(window, "Red: ");
        auto red_int_box = new IntBox<int>(window);
        red_int_box->set_editable(false);
        new Label(window, "Green: ");
        auto green_int_box = new IntBox<int>(window);
        green_int_box->set_editable(false);
        new Label(window, "Blue: ");
        auto blue_int_box = new IntBox<int>(window);
        blue_int_box->set_editable(false);
        new Label(window, "Alpha: ");
        auto alpha_int_box = new IntBox<int>(window);

        cp->set_callback([b,red_int_box,blue_int_box,green_int_box,alpha_int_box](const Color &c) {
            b->set_background_color(c);
            b->set_text_color(c.contrasting_color());
            int red = (int) (c.r() * 255.0f);
            red_int_box->set_value(red);
            int green = (int) (c.g() * 255.0f);
            green_int_box->set_value(green);
            int blue = (int) (c.b() * 255.0f);
            blue_int_box->set_value(blue);
            int alpha = (int) (c.w() * 255.0f);
            alpha_int_box->set_value(alpha);
        });

        perform_layout();

        /* All NanoGUI widgets are initialized at this point. Now
           create shaders to draw the main window contents.

           NanoGUI comes with a simple wrapper around OpenGL 3, which
           eliminates most of the tedious and error-prone shader and buffer
           object management.
        */

        m_render_pass = new RenderPass({ this });
        m_render_pass->set_clear_color(0, Color(0.3f, 0.3f, 0.32f, 1.f));

        m_shader = new Shader(
            m_render_pass,

            /* An identifying name */
            "a_simple_shader",

#if defined(NANOGUI_USE_OPENGL)
            R"(/* Vertex shader */
            #version 330
            uniform mat4 mvp;
            in vec3 position;
            void main() {
                gl_Position = mvp * vec4(position, 1.0);
            })",

            /* Fragment shader */
            R"(#version 330
            out vec4 color;
            uniform float intensity;
            void main() {
                color = vec4(vec3(intensity), 1.0);
            })"
#elif defined(NANOGUI_USE_GLES)
            R"(/* Vertex shader */
            precision highp float;
            uniform mat4 mvp;
            attribute vec3 position;
            void main() {
                gl_Position = mvp * vec4(position, 1.0);
            })",

            /* Fragment shader */
            R"(precision highp float;
            uniform float intensity;
            void main() {
                gl_FragColor = vec4(vec3(intensity), 1.0);
            })"
#elif defined(NANOGUI_USE_METAL)
            R"(using namespace metal;
            struct VertexOut {
                float4 position [[position]];
            };

            vertex VertexOut vertex_main(const device packed_float3 *position,
                                         constant float4x4 &mvp,
                                         uint id [[vertex_id]]) {
                VertexOut vert;
                vert.position = mvp * float4(position[id], 1.f);
                return vert;
            })",

            /* Fragment shader */
            R"(using namespace metal;
            fragment float4 fragment_main(const constant float &intensity) {
                return float4(intensity);
            })"
#endif
        );

        uint32_t indices[3*2] = {
            0, 1, 2,
            2, 3, 0
        };

        float positions[3*4] = {
            -1.f, -1.f, 0.f,
            1.f, -1.f, 0.f,
            1.f, 1.f, 0.f,
            -1.f, 1.f, 0.f
        };

        m_shader->set_buffer("indices", VariableType::UInt32, {3*2}, indices);
        m_shader->set_buffer("position", VariableType::Float32, {4, 3}, positions);
        m_shader->set_uniform("intensity", 0.5f);


    const char* glsl_version = "#version 130";

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(m_glfw_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);
    io.Fonts->AddFontFromFileTTF("../resources/Inconsolata-Regular.ttf", 14.0f*1.25);
    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);



    //TextEditor initialization
	// Load Fonts
	// (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
	ImVec4 clear_col = ImColor(114, 144, 154);

	///////////////////////////////////////////////////////////////////////
	// TEXT EDITOR SAMPLE

	// set your own known preprocessor symbols...
	static const char* ppnames[] = { "NULL", "PM_REMOVE",
		"ZeroMemory", "DXGI_SWAP_EFFECT_DISCARD", "D3D_FEATURE_LEVEL", "D3D_DRIVER_TYPE_HARDWARE", "WINAPI","D3D11_SDK_VERSION", "assert" };
	// ... and their corresponding values
	static const char* ppvalues[] = { 
		"#define NULL ((void*)0)", 
		"#define PM_REMOVE (0x0001)",
		"Microsoft's own memory zapper function\n(which is a macro actually)\nvoid ZeroMemory(\n\t[in] PVOID  Destination,\n\t[in] SIZE_T Length\n); ", 
		"enum DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD = 0", 
		"enum D3D_FEATURE_LEVEL", 
		"enum D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE  = ( D3D_DRIVER_TYPE_UNKNOWN + 1 )",
		"#define WINAPI __stdcall",
		"#define D3D11_SDK_VERSION (7)",
		" #define assert(expression) (void)(                                                  \n"
        "    (!!(expression)) ||                                                              \n"
        "    (_wassert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0) \n"
        " )"
		};

	for (int i = 0; i < sizeof(ppnames) / sizeof(ppnames[0]); ++i)
	{
		TextEditor::Identifier id;
		id.mDeclaration = ppvalues[i];
		lang.mPreprocIdentifiers.insert(std::make_pair(std::string(ppnames[i]), id));
	}

	// set your own identifiers
	static const char* identifiers[] = {
		"HWND", "HRESULT", "LPRESULT","D3D11_RENDER_TARGET_VIEW_DESC", "DXGI_SWAP_CHAIN_DESC","MSG","LRESULT","WPARAM", "LPARAM","UINT","LPVOID",
		"ID3D11Device", "ID3D11DeviceContext", "ID3D11Buffer", "ID3D11Buffer", "ID3D10Blob", "ID3D11VertexShader", "ID3D11InputLayout", "ID3D11Buffer",
		"ID3D10Blob", "ID3D11PixelShader", "ID3D11SamplerState", "ID3D11ShaderResourceView", "ID3D11RasterizerState", "ID3D11BlendState", "ID3D11DepthStencilState",
		"IDXGISwapChain", "ID3D11RenderTargetView", "ID3D11Texture2D", "TextEditor" };
	static const char* idecls[] = 
	{
		"typedef HWND_* HWND", "typedef long HRESULT", "typedef long* LPRESULT", "struct D3D11_RENDER_TARGET_VIEW_DESC", "struct DXGI_SWAP_CHAIN_DESC",
		"typedef tagMSG MSG\n * Message structure","typedef LONG_PTR LRESULT","WPARAM", "LPARAM","UINT","LPVOID",
		"ID3D11Device", "ID3D11DeviceContext", "ID3D11Buffer", "ID3D11Buffer", "ID3D10Blob", "ID3D11VertexShader", "ID3D11InputLayout", "ID3D11Buffer",
		"ID3D10Blob", "ID3D11PixelShader", "ID3D11SamplerState", "ID3D11ShaderResourceView", "ID3D11RasterizerState", "ID3D11BlendState", "ID3D11DepthStencilState",
		"IDXGISwapChain", "ID3D11RenderTargetView", "ID3D11Texture2D", "class TextEditor" };
	for (int i = 0; i < sizeof(identifiers) / sizeof(identifiers[0]); ++i)
	{
		TextEditor::Identifier id;
		id.mDeclaration = std::string(idecls[i]);
		lang.mIdentifiers.insert(std::make_pair(std::string(identifiers[i]), id));
	}
	editor.SetLanguageDefinition(lang);
	//editor.SetPalette(TextEditor::GetLightPalette());

	// error markers
	TextEditor::ErrorMarkers markers;
	markers.insert(std::make_pair<int, std::string>(6, "Example error here:\nInclude file not found: \"TextEditor.h\""));
	markers.insert(std::make_pair<int, std::string>(41, "Another example error"));
	editor.SetErrorMarkers(markers);

	// "breakpoint" markers
	//TextEditor::Breakpoints bpts;
	//bpts.insert(24);
	//bpts.insert(47);
	//editor.SetBreakpoints(bpts);

//	static const char* fileToEdit = "test.cpp";

	{
		std::ifstream t(fileToEdit);
		if (t.good())
		{
			std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
			editor.SetText(str);
		}
	}



    }

    virtual bool keyboard_event(int key, int scancode, int action, int modifiers) {
        if (Screen::keyboard_event(key, scancode, action, modifiers))
            return true;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            set_visible(false);
            return true;
        }
        return false;
    }

    virtual void draw(NVGcontext *ctx) {
        /* Animate the scrollbar */
        m_progress->set_value(std::fmod((float) glfwGetTime() / 10, 1.0f));

        /* Draw the user interface */
        Screen::draw(ctx);
    }

    virtual void draw_contents() {
        
        Matrix4f mvp = Matrix4f::scale(Vector3f(
                           (float) m_size.y() / (float) m_size.x() * 0.25f, 0.25f, 0.25f)) *
                       Matrix4f::rotate(Vector3f(0, 0, 1), (float) glfwGetTime());

        m_shader->set_uniform("mvp", mvp);

        m_render_pass->resize(framebuffer_size());
        m_render_pass->begin();

        m_shader->begin();
        m_shader->draw_array(Shader::PrimitiveType::Triangle, 0, 6, true);
        m_shader->end();

        m_render_pass->end();
        

        bool show_demo_window = true;
        bool show_another_window = false;
        int display_w, display_h;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


        #if 1
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();
        {
		auto cpos = editor.GetCursorPosition();
		ImGui::Begin("Text Editor Demo", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
		ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save"))
				{
					auto textToSave = editor.GetText();
					/// save text....
				}
				if (ImGui::MenuItem("Quit", "Alt-F4")) 
                ;//break;
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				bool ro = editor.IsReadOnly();
				if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
					editor.SetReadOnly(ro);
				ImGui::Separator();

				if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor.CanUndo()))
					editor.Undo();
				if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
					editor.Redo();

				ImGui::Separator();

				if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
					editor.Copy();
				if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))
					editor.Cut();
				if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
					editor.Delete();
				if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
					editor.Paste();

				ImGui::Separator();

				if (ImGui::MenuItem("Select all", nullptr, nullptr))
					editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Dark palette"))
					editor.SetPalette(TextEditor::GetDarkPalette());
				if (ImGui::MenuItem("Light palette"))
					editor.SetPalette(TextEditor::GetLightPalette());
				if (ImGui::MenuItem("Retro blue palette"))
					editor.SetPalette(TextEditor::GetRetroBluePalette());
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
			editor.IsOverwrite() ? "Ovr" : "Ins",
			editor.CanUndo() ? "*" : " ",
			editor.GetLanguageDefinition().mName.c_str(), fileToEdit);

		editor.Render("TextEditor");

        }

        {
            static float f = 0.0f;
            static int counter = 0;

            //ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }


        // Rendering
        ImGui::Render();

        glfwGetFramebufferSize(m_glfw_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        //glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        //glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        //glfwSwapBuffers(m_glfw_window);
        #endif

    }
private:
    ProgressBar *m_progress;
    ref<Shader> m_shader;
    ref<RenderPass> m_render_pass;

    using ImageHolder = std::unique_ptr<uint8_t[], void(*)(void*)>;
    std::vector<std::pair<ref<Texture>, ImageHolder>> m_images;
    int m_current_image;
};

int main(int /* argc */, char ** /* argv */) {
    try {
        nanogui::init();

        /* scoped variables */ {
            ref<ExampleApplication> app = new ExampleApplication();
            app->dec_ref();
            app->draw_all();
            app->set_visible(true);
            nanogui::mainloop(1 / 60.f * 1000);
        }

        nanogui::shutdown();
    } catch (const std::exception &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        #if defined(_WIN32)
            MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
        #else
            std::cerr << error_msg << std::endl;
        #endif
        return -1;
    } catch (...) {
        std::cerr << "Caught an unknown error!" << std::endl;
    }

    return 0;
}
