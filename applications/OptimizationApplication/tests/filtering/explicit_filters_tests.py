import numpy

import KratosMultiphysics as Kratos
import KratosMultiphysics.OptimizationApplication as KratosOA
from KratosMultiphysics.testing.utilities import ReadModelPart

# Import KratosUnittest
import KratosMultiphysics.KratosUnittest as kratos_unittest

class TestExplicitVertexMorphingFilter(kratos_unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.model = Kratos.Model()
        cls.model_part = cls.model.CreateModelPart("test")
        cls.model_part.AddNodalSolutionStepVariable(Kratos.VELOCITY)
        cls.model_part.AddNodalSolutionStepVariable(Kratos.ACCELERATION)
        with kratos_unittest.WorkFolderScope(".", __file__):
            ReadModelPart("solid", cls.model_part)

        for node in cls.model_part.Nodes:
            node.SetSolutionStepValue(Kratos.VELOCITY, Kratos.Array3([node.Id, node.Id + 1, node.Id + 2]))

        for condition in cls.model_part.Conditions:
            condition.SetValue(Kratos.VELOCITY, Kratos.Array3([condition.Id, condition.Id + 1, condition.Id + 2]))

        for element in cls.model_part.Elements:
            element.SetValue(Kratos.VELOCITY, Kratos.Array3([element.Id, element.Id + 1, element.Id + 2]))

    def test_NodalExplicitVertexMorphingFilter(self):
        radius = 2.0
        filter = KratosOA.NodalExplicitVertexMorphingFilter(self.model_part, "linear", 1000)

        filter_radius = Kratos.Expression.NodalExpression(self.model_part)
        Kratos.Expression.LiteralExpressionIO.SetData(filter_radius, radius)
        filter.SetFilterRadius(filter_radius)

        unfiltered_field = Kratos.Expression.NodalExpression(self.model_part)
        Kratos.Expression.VariableExpressionIO.Read(unfiltered_field, Kratos.VELOCITY, True)
        filtered_data = filter.FilterField(unfiltered_field)

        ref_values = numpy.array([
                [ 7.195467517797389,   8.19546751779739,    9.195467517797388 ],
                [ 7.318570814762066,   8.318570814762067,   9.318570814762067 ],
                [ 7.55905455007206,    8.55905455007206,    9.559054550072059 ],
                [ 7.6821578470367395,  8.68215784703674,    9.68215784703674  ],
                [ 7.88351476956496,    8.883514769564961,   9.883514769564961 ],
                [ 8.006618066529638,   9.00661806652964,   10.00661806652964  ],
                [ 8.247101801839632,   9.247101801839634,  10.247101801839634 ],
                [ 8.37020509880431,    9.37020509880431,   10.37020509880431  ],
                [ 7.6458863554962315,  8.645886355496232,   9.645886355496232 ],
                [ 8.210354586583765,   9.210354586583765,  10.210354586583765 ],
                [ 7.822178630754171,   8.822178630754172,   9.822178630754172 ],
                [ 8.134818666059648,   9.134818666059648,  10.134818666059646 ],
                [ 8.061427954957628,   9.06142795495763,   10.06142795495763  ],
                [ 7.996325696590016,   8.996325696590018,   9.996325696590018 ]
            ])

        self.assertAlmostEqual(numpy.linalg.norm(filtered_data.Evaluate() - ref_values), 0.0, 12)

        unfiltered_field = filter.UnFilterField(unfiltered_field)

        ref_values = numpy.array([
                [ 6.1450533208134175,  7.053380421583984,   7.961707522354552 ],
                [ 6.27580490608711,    7.184132006857679,   8.092459107628246 ],
                [ 6.5009920647790755,  7.409319165549642,   8.31764626632021  ],
                [ 6.631743650052767,   7.540070750823336,   8.448397851593903 ],
                [ 6.825452284271976,   7.733779385042544,   8.642106485813112 ],
                [ 6.956203869545668,   7.8645309703162365,  8.772858071086805 ],
                [ 7.181391028237634,   8.0897181290082,     8.998045229778768 ],
                [ 7.312142613511326,   8.220469714281892,   9.12879681505246  ],
                [ 8.134737444400715,   9.256967976706626,  10.379198509012534 ],
                [ 8.821578288432477,   9.943808820738388,  11.066039353044298 ],
                [ 8.34162287289471,    9.463853405200622,  10.586083937506533 ],
                [ 8.7154492146723,     9.837679746978212,  10.959910279284122 ],
                [ 8.626761926952252,   9.748992459258163,  10.871222991564075 ],
                [ 8.531066515348583,   9.653297047654492,  10.775527579960402 ]
            ])

        self.assertAlmostEqual(numpy.linalg.norm(unfiltered_field.Evaluate() - ref_values), 0.0, 12)

    def test_ConditionExplicitVertexMorphingFilter(self):
        radius = 1.0
        filter = KratosOA.ConditionExplicitVertexMorphingFilter(self.model_part, "linear", 1000)

        filter_radius = Kratos.Expression.ConditionExpression(self.model_part)
        Kratos.Expression.LiteralExpressionIO.SetData(filter_radius, radius)
        filter.SetFilterRadius(filter_radius)

        unfiltered_field = Kratos.Expression.ConditionExpression(self.model_part)
        Kratos.Expression.VariableExpressionIO.Read(unfiltered_field, Kratos.VELOCITY)
        filtered_data = filter.FilterField(unfiltered_field)

        ref_values = numpy.array([
            [ 6.758330624355697,  7.758330624355698,  8.758330624355697],
            [ 8.563643579842978,  9.563643579842978, 10.563643579842978],
            [ 7.694047957950673,  8.694047957950673,  9.694047957950675],
            [ 8.292863410975192,  9.292863410975192, 10.292863410975192],
            [ 7.268144993577022,  8.268144993577023,  9.268144993577023],
            [ 9.966790169575301, 10.9667901695753,   11.966790169575303],
            [ 8.919588772181047,  9.919588772181049, 10.919588772181045],
            [10.194739547987977, 11.194739547987977, 12.194739547987975],
            [ 9.497741144178006, 10.497741144178008, 11.497741144178008],
            [12.302092912247636, 13.302092912247637, 14.302092912247636],
            [11.254891514853382, 12.254891514853382, 13.254891514853384],
            [12.930118217431987, 13.930118217431989, 14.930118217431987],
            [ 9.092184727756681, 10.092184727756681, 11.092184727756681],
            [ 9.919698370675327, 10.919698370675325, 11.919698370675327],
            [11.454569130368588, 12.454569130368586, 13.454569130368588],
            [12.486120423462921, 13.486120423462921, 14.486120423462921],
            [10.595147944188897, 11.595147944188895, 12.595147944188895],
            [11.636465499406407, 12.63646549940641,  13.636465499406409],
            [13.171336259099668, 14.17133625909967,  15.171336259099666],
            [14.686329600585715, 15.686329600585713, 16.686329600585715]
        ])
        self.assertAlmostEqual(numpy.linalg.norm(filtered_data.Evaluate() - ref_values), 0.0, 12)

        unfiltered_field = filter.UnFilterField(unfiltered_field)

        ref_values = numpy.array([
            [ 7.316414814468337,  8.371362020425924,  9.426309226383509],
            [ 9.375280941019922, 10.430228146977512, 11.485175352935098],
            [ 8.385218328952282,  9.440165534909871, 10.495112740867459],
            [ 8.998285386811697, 10.053232592769284, 11.108179798726868],
            [ 7.899464266489179,  8.9558526032111,   10.012240939933024],
            [10.345792351698519, 11.355035992080753, 12.364279632462988],
            [ 9.247050281724967, 10.256293922107202, 11.265537562489438],
            [ 9.055586047605622,  9.925763224161642, 10.795940400717663],
            [10.316194011167912, 11.372582347889836, 12.428970684611759],
            [12.71538890547423,  13.724632545856466, 14.7338761862387  ],
            [11.616646835500681, 12.625890475882912, 13.635134116265148],
            [11.412405222956844, 12.282582399512862, 13.152759576068885],
            [ 9.93934955314518,  10.995737889867105, 12.052126226589026],
            [10.277743232758645, 11.286986873140881, 12.296230513523117],
            [11.837283869482674, 12.846527509864911, 13.855771150247142],
            [11.00746278025307,  11.877639956809091, 12.74781713336511 ],
            [11.61071477112295,  12.667103107844873, 13.723491444566795],
            [12.067010452535815, 13.076254092918049, 14.085497733300286],
            [13.626551089259843, 14.635794729642079, 15.645038370024313],
            [12.950156857571614, 13.820334034127635, 14.690511210683656]
        ])

        self.assertAlmostEqual(numpy.linalg.norm(unfiltered_field.Evaluate() - ref_values), 0.0, 12)

    def test_ElementExplicitVertexMorphingFilter(self):
        radius = 1.0
        filter = KratosOA.ElementExplicitVertexMorphingFilter(self.model_part, "linear", 1000)

        filter_radius = Kratos.Expression.ElementExpression(self.model_part)
        Kratos.Expression.LiteralExpressionIO.SetData(filter_radius, radius)
        filter.SetFilterRadius(filter_radius)

        unfiltered_field = Kratos.Expression.ElementExpression(self.model_part)
        Kratos.Expression.VariableExpressionIO.Read(unfiltered_field, Kratos.VELOCITY)
        filtered_data = filter.FilterField(unfiltered_field)

        ref_values = numpy.array([
            [11.55444743868707,  12.554447438687067, 13.554447438687067],
            [11.918041958443546, 12.918041958443546, 13.918041958443544],
            [11.928193300444164, 12.928193300444164, 13.928193300444162],
            [12.282886864969507, 13.282886864969505, 14.282886864969505],
            [11.5471098313716,   12.547109831371602, 13.547109831371603],
            [11.246916707604543, 12.24691670760454,  13.24691670760454 ],
            [12.36654479019958,  13.36654479019958,  14.366544790199583],
            [10.896867880093462, 11.896867880093463, 12.896867880093463],
            [11.655383133574905, 12.655383133574908, 13.655383133574906],
            [11.455040010196624, 12.455040010196623, 13.45504001019662 ],
            [12.511738531311057, 13.511738531311059, 14.511738531311057],
            [12.231281395258035, 13.231281395258033, 14.231281395258035],
            [12.221777211365726, 13.221777211365728, 14.221777211365726],
            [13.052716422133567, 14.052716422133567, 15.052716422133567],
            [13.436252830633203, 14.436252830633203, 15.436252830633203],
            [13.168849831216496, 14.1688498312165,   15.168849831216498],
            [11.369765135635856, 12.369765135635854, 13.369765135635857],
            [12.375621221163348, 13.375621221163348, 14.375621221163346],
            [11.584368222275419, 12.584368222275417, 13.584368222275417],
            [12.41708255783569,  13.417082557835688, 14.41708255783569 ],
            [12.22618075312927,  13.226180753129269, 14.226180753129274],
            [12.88371497110807,  13.883714971108073, 14.883714971108072],
            [12.180052031010879, 13.180052031010879, 14.180052031010879],
            [13.302694861592688, 14.302694861592686, 15.302694861592688]
        ])

        self.assertAlmostEqual(numpy.linalg.norm(filtered_data.Evaluate() - ref_values), 0.0, 12)

        unfiltered_field = filter.UnFilterField(unfiltered_field)

        ref_values = numpy.array([
            [15.908318123609645, 17.24048749846822,  18.572656873326803],
            [16.440677246879734, 17.772846621738307, 19.10501599659689 ],
            [16.419385087896814, 17.75155446275539,  19.083723837613974],
            [16.937981621392954, 18.270150996251527, 19.602320371110107],
            [10.26225367959307,  11.130532115226055, 11.998810550859039],
            [ 9.950175152551044, 10.818453588184028, 11.686732023817012],
            [10.987343964504804, 11.855622400137786, 12.723900835770776],
            [ 9.700040412097264, 10.570579578405836, 11.441118744714412],
            [10.401739946325977, 11.27001838195896,  12.138296817591948],
            [10.189511419672726, 11.057789855305709, 11.926068290938693],
            [11.16375067014589,  12.03202910577887,  12.900307541411857],
            [10.987123524707476, 11.85766269101605,  12.728201857324622],
            [10.91444716871862,  11.784986335027192, 12.655525501335767],
            [11.716613964243365, 12.58489239987635,  13.453170835509338],
            [12.081592385431737, 12.95213155174031,  13.82267071804889 ],
            [11.801534408796691, 12.669812844429677, 13.538091280062662],
            [11.988984426335266, 13.019351720118708, 14.04971901390215 ],
            [13.154439295763114, 14.18480658954655,  15.215173883329989],
            [12.243374187559441, 13.273741481342878, 14.30410877512632 ],
            [13.149939428720137, 14.180306722503579, 15.21067401628702 ],
            [12.953989100692866, 13.9843563944763,   15.014723688259744],
            [13.65183646976496,  14.682203763548397, 15.712571057331841],
            [12.845064596181807, 13.875431889965247, 14.905799183748686],
            [14.149883718414666, 15.1802510121981,   16.210618305981544]
        ])

        self.assertAlmostEqual(numpy.linalg.norm(unfiltered_field.Evaluate() - ref_values), 0.0, 12)


if __name__ == "__main__":
    Kratos.Tester.SetVerbosity(Kratos.Tester.Verbosity.PROGRESS)  # TESTS_OUTPUTS
    kratos_unittest.main()