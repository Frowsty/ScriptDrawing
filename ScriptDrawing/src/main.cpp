#define OLC_PGE_APPLICATION
#define OLC_PGEX_FUI
#include "headers/olcPixelGameEngine.h"
#include "headers/olcPGEX_FrostUI.h"
#include "headers/scripter.h"

bool first_run = true;
bool settings_finished = false;

class ConsoleApp : public olc::PixelGameEngine
{
public:
    ConsoleApp()
    {
        sAppName = "Script painting";
    }

    uint32_t paint_layer;

    ScriptHandler scripter;

    enum class DrawType
    {
        RECT,
        CIRCLE,
        LINE
    };

    bool running = true;


    olc::vi2d console_size = { 1280, 200 };
    std::vector<std::pair<DrawType, std::vector<int>>> draw_calls;
    int draw_call_size = 0;

    olc::Pixel pencil_color{ 0, 0, 0};
    int32_t pencil_size = 2;
    bool trailing_pencil = false;
    float trailing_pencil_speed = 1.0f;

    bool clear_canvas = false;
    olc::Pixel canvas_color{ 255, 255, 255 };

    std::vector<std::string> help_command =
    {
        "print(text) - prints text to console",
        "draw_rect(x_pos, y_pos, width, height, red, green, blue) - draws a rect with given size and color",
        "draw_circle(x_pos, y_pos, radius, red, green, blue) - draws a circle with given size and color",
        "draw_line(x1_pos, y1_pos, x2_pos, y2_pos, red, gree, blue) - draws a line with given color",
        "set_pencil_color(red, green, blue) - sets the color of the pencil",
        "set_pencil_size(size) - sets the size of the pencil",
        "set_trailing_pencil(true/false) - sets pencil to trail behind the cursor",
        "set_trailing_speed(speed) = sets pencil trail speed",
        "clear_canvas() - clears the drawing canvas",
        "clear_console() - clears the console",
        "set_canvas_color() - clears the drawing canvas and changes color",
        "exit() - exits the application"
    };
    int32_t help_iterations = 0;
    bool help_executed = true;

    olc::FrostUI gui;
    std::vector<std::string> commands
    {
        "help",
        "print",
        "draw_rect",
        "draw_circle",
        "draw_line",
        "set_pencil_color",
        "set_pencil_size",
        "set_trailing_pencil",
        "set_trailing_speed",
        "clear_canvas",
        "clear_console",
        "set_canvas_color",
        "exit"
    };

    olc::vf2d cursor_pos = GetMousePos();
    olc::vf2d pen_pos;

    // THANK YOU MEGAREV YET AGAIN!
    void pen_drawing()
    {
        if (cursor_pos.y > 0 && cursor_pos.y < 520)
        {
            if (GetMouse(0).bPressed)
                pen_pos = cursor_pos;
            if (GetMouse(0).bHeld)
            {
                //pen_drawing();
                //pen_drawing(GetMouseX(), GetMouseY(), last_mouse_pos.x, last_mouse_pos.y, pencil_size, pencil_color);
                if (trailing_pencil)
                {
                    for (int n = 0; n < 20; n++)
                    {
                        pen_pos += ((cursor_pos - pen_pos) * trailing_pencil_speed) * GetElapsedTime();
                        FillCircle(pen_pos, pencil_size / 2, pencil_color);
                    }
                }
                else
                {
                    int x1 = pen_pos.x, y1 = pen_pos.y, x2 = cursor_pos.x, y2 = cursor_pos.y;
                    olc::vf2d direction = { (float)(x2 - x1), (float)(y2 - y1) };
                    olc::vf2d axis_proj = direction.perp();
                    float len = axis_proj.mag();
                    if (len > 0.001f) axis_proj /= len;
                    axis_proj *= pencil_size;

                    olc::vi2d t1 = olc::vi2d(x1, y1) + axis_proj / 2;
                    olc::vi2d t2 = t1 - axis_proj;
                    olc::vi2d t3 = olc::vi2d(x2, y2) + axis_proj / 2;
                    olc::vi2d t4 = t3 - axis_proj;

                    FillTriangle(t1.x, t1.y, t2.x, t2.y, t3.x, t3.y, pencil_color);
                    FillTriangle(t2.x, t2.y, t3.x, t3.y, t4.x, t4.y, pencil_color);

                    FillCircle(x1, y1, pencil_size / 2, pencil_color);
                    FillCircle(x2, y2, pencil_size / 2, pencil_color);
                }
            }
        }
    }

    void draw_objects()
    {
        SetDrawTarget(paint_layer);

        if (clear_canvas)
        {
            Clear(canvas_color);
            draw_calls.clear();
            clear_canvas = false;
        }

        if (draw_call_size != draw_calls.size())
        {
            for (auto& call : draw_calls)
            {
                olc::Pixel color;
                switch (call.first)
                {
                case DrawType::RECT:
                    color = olc::Pixel(call.second[4], call.second[5], call.second[6]);
                    FillRect({ call.second[0], call.second[1] }, { call.second[2], call.second[3] }, color);
                    break;
                case DrawType::CIRCLE:
                    color = olc::Pixel(call.second[3], call.second[4], call.second[5]);
                    FillCircle(olc::vi2d(call.second[0], call.second[1]), call.second[2] / 2, color);
                    break;
                case DrawType::LINE:
                    color = olc::Pixel(call.second[4], call.second[5], call.second[6]);
                    DrawLine(call.second[0], call.second[1], call.second[2], call.second[3], color);
                    break;
                }
            }
            draw_call_size = draw_calls.size();
        }

        pen_drawing();
        if (!trailing_pencil)
            pen_pos = cursor_pos;

        EnableLayer(paint_layer, true);
        SetDrawTarget(nullptr);
    }
public:
    bool OnUserCreate() override
    {
        paint_layer = CreateLayer();

        SetDrawTarget(paint_layer);
        Clear(olc::WHITE);
        EnableLayer(paint_layer, true);
        SetDrawTarget(nullptr);

        scripter.add_keywords(commands);

        gui.add_console("console", "ScriptDrawing Console", { 0, 520 }, console_size, 30);
        gui.find_element("console")->scale_text({ 1.75f, 1.75f });
        gui.find_element("console")->add_command_handler([&](std::string& cmd, std::string* return_msg)
            {
                // check the syntax and if the keyword exists, store results into keyword
                auto keyword = scripter.check_syntax(cmd);
                /*
                    keyword.first = "command" (if passed the syntax check) / "error" (if failed the syntax check)
                    keyword.second = if (keyword.first == true) 
                                        keyword.second = command 
                                     else
                                        keyword.second = error_code
                */
                if (keyword.first != "error")
                {
                    auto data = scripter.extract_data(keyword.second);
                    bool did_execute = true;
                    if (keyword.first == "print")
                        *return_msg = keyword.second;
                    else if (keyword.first == "clear_console")
                        gui.find_element("console")->clear_console();
                    else if (keyword.first == "clear_canvas")
                    {
                        clear_canvas = true;
                        *return_msg = "Canvas cleared";
                    }
                    else if (keyword.first == "help")
                    {
                        help_executed = true;
                        *return_msg = keyword.first + " executed";
                    }
                    else if (keyword.first == "exit")
                    {
                        running = false;
                        *return_msg = "exiting...";
                    }
                    else if (keyword.first == "set_trailing_pencil")
                    {
                        if (keyword.second == "true")
                            trailing_pencil = true;
                        else if (keyword.second == "false")
                            trailing_pencil = false;

                        *return_msg = keyword.first + " executed";
                    }
                    else
                        did_execute = false;

                    // commands that require data to have received data from scripter
                    if (data.size() > 0 && !did_execute)
                    {
                        if (keyword.first == "draw_rect" && data.size() == 7)
                        {
                            draw_calls.push_back(std::make_pair(DrawType::RECT, data));
                            *return_msg = keyword.first + " executed";
                        }
                        else if (keyword.first == "draw_circle" && data.size() == 6)
                        {
                            draw_calls.push_back(std::make_pair(DrawType::CIRCLE, data));
                            *return_msg = keyword.first + " executed";
                        }
                        else if (keyword.first == "draw_line" && data.size() == 7)
                        {
                            draw_calls.push_back(std::make_pair(DrawType::LINE, data));
                            *return_msg = keyword.first + " executed";
                        }
                        else if (keyword.first == "set_pencil_color" && data.size() == 3)
                        {
                            pencil_color.r = data[0];
                            pencil_color.g = data[1];
                            pencil_color.b = data[2];
                            *return_msg = keyword.first + " executed";
                        }
                        else if (keyword.first == "set_pencil_size" && data.size() == 1)
                        {
                            pencil_size = data[0];
                            *return_msg = keyword.first + " executed";
                        }
                        else if (keyword.first == "set_trailing_speed" && data.size() == 1)
                        {
                            trailing_pencil_speed = data[0] * 0.1;
                            *return_msg = keyword.first + " executed";
                        }
                        else if (keyword.first == "set_canvas_color" && data.size() == 3)
                        {
                            clear_canvas = true;
                            canvas_color.r = data[0];
                            canvas_color.g = data[1];
                            canvas_color.b = data[2];
                            *return_msg = keyword.first + " executed";
                        }
                        else
                            *return_msg = "Parameter amount does not match the function call";
                    }
                    else if (!did_execute)
                        *return_msg = "Failed to retreive data from parameters";
                }
                else
                    *return_msg = keyword.first + ": " + keyword.second;
            });

        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        cursor_pos = GetMousePos();
        Clear(olc::BLANK);

        if (help_executed && help_iterations <= help_command.size() - 1)
        {
            gui.find_element("console")->add_command_entry(help_command[help_iterations]);

            help_iterations++;
        }
        else
        {
            help_executed = false;
            help_iterations = 0;
        }

        gui.run();

        draw_objects();

        return running;
    }
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    ConsoleApp* console_app = new ConsoleApp;

    if (console_app->Construct(1280, 720, 1, 1, false))
        console_app->Start();

    return 0;
}