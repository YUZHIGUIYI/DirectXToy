//
// Created by ZZK on 2024/3/13.
//

#pragma once

#include <Toy/toy.h>
#include <Toy/Runtime/application.h>

namespace toy::editor
{
    struct EditorApplication final : public runtime::Application
    {
    public:
        EditorApplication() = default;

        ~EditorApplication() override = default;

        void start() override;

        void setup() override;

        void stop() override;

    private:

    };
}
