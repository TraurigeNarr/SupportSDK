set_name = "render_testing"

mesh = {
	resource_name = "SimpleBox"
	path = "..\..\..\Samples\Resources\Models\Box.obj"
	vertices_usage = "static"
	indices_usage = "static"
}

texture = {
	resource_name = "nano_arm_diffuse_tex"
	path = "..\..\..\Samples\Resources\Textures\nanosuit\arm_dif.png"
}

texture = {
	resource_name = "nano_arm_specular_tex"
	path = "..\..\..\Samples\Resources\Textures\nanosuit\arm_showroom_spec.png"
}

texture = {
	resource_name = "nano_body_diffuse_tex"
	path = "..\..\..\Samples\Resources\Textures\nanosuit\body_dif.png"
}

texture = {
	resource_name = "nano_body_specular_tex"
	path = "..\..\..\Samples\Resources\Textures\nanosuit\body_showroom_spec.png"
}

texture = {
	resource_name = "nano_hand_diffuse_tex"
	path = "..\..\..\Samples\Resources\Textures\nanosuit\hand_dif.png"
}

texture = {
	resource_name = "nano_hand_specular_tex"
	path = "..\..\..\Samples\Resources\Textures\nanosuit\hand_showroom_spec.png"
}

texture = {
	resource_name = "nano_helmet_diffuse_tex"
	path = "..\..\..\Samples\Resources\Textures\nanosuit\helmet_dif.png"
}

texture = {
	resource_name = "nano_helmet_specular_tex"
	path = "..\..\..\Samples\Resources\Textures\nanosuit\helmet_showroom_spec.png"
}

texture = {
	resource_name = "nano_glass_diffuse_tex"
	path = "..\..\..\Samples\Resources\Textures\nanosuit\glass_dif.png"
}

texture = {
	resource_name = "nano_glass_specular_tex"
	path = "..\..\..\Samples\Resources\Textures\nanosuit\glass_ddn.png"
}

texture = {
	resource_name = "nano_leg_diffuse_tex"
	path = "..\..\..\Samples\Resources\Textures\nanosuit\leg_dif.png"
}

texture = {
	resource_name = "nano_leg_specular_tex"
	path = "..\..\..\Samples\Resources\Textures\nanosuit\leg_showroom_spec.png"
}

texture = {
	resource_name = "box_diffuse"
	path = "..\..\..\Samples\Resources\Textures\Box texture.png"
}

shader_program = {
	resource_name = "SimpleShader"
	shader = {
		type = "vertex"
		path = "..\..\..\Samples\Resources\Shaders\Sample.vertexshader"
	}
	shader = {
		type = "fragment"
		path = "..\..\..\Samples\Resources\Shaders\Sample.fragmentshader"
	}
}

shader_program = {
	resource_name = "SuitShader"
	shader = {
		type = "vertex"
		path = "..\..\..\Samples\Resources\Shaders\Suit.vertexshader"
	}
	shader = {
		type = "fragment"
		path = "..\..\..\Samples\Resources\Shaders\Suit.fragmentshader"
	}
}

shader_program = {
	resource_name = "TextShader"
	shader = {
		type = "vertex"
		path = "..\..\..\Samples\Resources\Shaders\TextShader.vertexshader"
	}
	shader = {
		type = "fragment"
		path = "..\..\..\Samples\Resources\Shaders\TextShader.fragmentshader"
	}
}

material = {
	resource_name = "Sample_material"
	path = "..\..\..\Samples\Resources\Materials\sample.material"
}

material = {
	resource_name = "Arms_material"
	path = "..\..\..\Samples\Resources\Materials\nanosuit\arms.material"
}

material = {
	resource_name = "Body_material"
	path = "..\..\..\Samples\Resources\Materials\nanosuit\body.material"
}

material = {
	resource_name = "Hands_material"
	path = "..\..\..\Samples\Resources\Materials\nanosuit\hands.material"
}

material = {
	resource_name = "Helmet_material"
	path = "..\..\..\Samples\Resources\Materials\nanosuit\helmet.material"
}

material = {
	resource_name = "Legs_material"
	path = "..\..\..\Samples\Resources\Materials\nanosuit\legs.material"
}

material = {
	resource_name = "Visor_material"
	path = "..\..\..\Samples\Resources\Materials\nanosuit\visor.material"
}

model = {
	resource_name = "Nanosuit"
	path = "..\..\..\Samples\Resources\Models\nanosuit\nanosuit.obj"
	description_path = "..\..\..\Samples\Resources\Models\nanosuit\nanosuit.model"
	vertices_usage = "static"
	indices_usage = "static"
}

font = {
	resource_name = "Arial_en"
	path = "..\..\..\Samples\Resources\Fonts\arial.ttf"
	codes = L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
	pixel_width = 0
	pixel_height = 48
}

font = {
	resource_name = "Arial_ru"
	path = "..\..\..\Samples\Resources\Fonts\arial.ttf"
	codes = L" !\"#$%&'()*+,-./0123456789:;<=>?@АБВГДАЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЬЭЮЯ[\\]^_`абвгдежзийклмнопрстуфхцчшщъыьэюя{|}~"
	pixel_width = 0
	pixel_height = 48
}
