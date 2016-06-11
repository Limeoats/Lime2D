#ifndef IMGUI_SFML_RENDERING_BACKEND
#define IMGUI_SFML_RENDERING_BACKEND
#include <vector>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <iostream>
#include <memory>
namespace ImGui
{
    namespace ImImpl
    {
        static sf::RenderTarget* ImImpl_rtarget;
        static sf::Texture* ImImpl_fontTex;

        static void ImImpl_RenderDrawLists(ImDrawData* draw_data)
        {
            if (draw_data->CmdListsCount == 0)
                return;

            ImImpl_rtarget->pushGLStates();

            GLint last_texture;
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
            glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_SCISSOR_TEST);
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glEnableClientState(GL_COLOR_ARRAY);
            glEnable(GL_TEXTURE_2D);

            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            glOrtho(0.0f, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, 0.0f, -1.0f, +1.0f);
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();

            sf::RenderStates states;
            states.blendMode = sf::BlendMode(sf::BlendMode::SrcAlpha, sf::BlendMode::OneMinusSrcAlpha);

            #define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
            for (int n = 0; n < draw_data->CmdListsCount; n++)
            {
                const ImDrawList* cmd_list = draw_data->CmdLists[n];
                const unsigned char* vtx_buffer = (const unsigned char*)&cmd_list->VtxBuffer.front();
                const ImDrawIdx* idx_buffer = &cmd_list->IdxBuffer.front();
                glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, pos)));
                glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, uv)));
                glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, col)));

                for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++)
                {
                    const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                    if (pcmd->UserCallback)
                    {
                        pcmd->UserCallback(cmd_list, pcmd);
                    }
                    else
                    {
                        sf::Vector2u win_size = ImImpl_rtarget->getSize();
                        sf::Texture::bind(ImImpl::ImImpl_fontTex);
                        glScissor((int)pcmd->ClipRect.x, (int)(win_size.y - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                        glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, GL_UNSIGNED_SHORT, idx_buffer);
                    }
                    idx_buffer += pcmd->ElemCount;
                }
            }
            #undef OFFSETOF

            // Restore modified state
            glDisableClientState(GL_COLOR_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glDisableClientState(GL_VERTEX_ARRAY);
            glBindTexture(GL_TEXTURE_2D, last_texture);
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glPopAttrib();

            ImImpl_rtarget->popGLStates();

            ImImpl_rtarget->resetGLStates();
        }
    }
    namespace SFML
    {
        static void SetRenderTarget(sf::RenderTarget& target){ImImpl::ImImpl_rtarget=&target;}
        static void InitImGuiRendering()
        {
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = ImVec2(float(ImImpl::ImImpl_rtarget->getSize().x), float(ImImpl::ImImpl_rtarget->getSize().y));
            io.RenderDrawListsFn = ImImpl::ImImpl_RenderDrawLists;
            unsigned char* pixels;
            int width, height;
            io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
            ImImpl::ImImpl_fontTex = new sf::Texture;
            ImImpl::ImImpl_fontTex->create(width, height);
            ImImpl::ImImpl_fontTex->update(pixels);
            io.Fonts->TexID = (void*)&ImImpl::ImImpl_fontTex;
            io.Fonts->ClearInputData();
            io.Fonts->ClearTexData();
        }
        static void UpdateImGuiRendering()
        {
                ImGuiIO& io = ImGui::GetIO();
                io.DisplaySize = ImVec2(float(ImImpl::ImImpl_rtarget->getSize().x), float(ImImpl::ImImpl_rtarget->getSize().y));
        }
        static void Shutdown()
        {
            ImGuiIO& io = ImGui::GetIO();
            io.Fonts->TexID = nullptr;
            delete ImImpl::ImImpl_fontTex;

            ImImpl::ImImpl_rtarget = nullptr;
            ImGui::Shutdown();
        }
    }
}
#endif
